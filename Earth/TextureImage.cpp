#include "Logger.h"
#include <algorithm>
#include "StagingBuffer.h"
#define STB_IMAGE_IMPLEMENTATION

#include "TextureImage.h"

// public:

TextureImage::TextureImage(Device *pDevice, std::vector<std::string> filenames, uint32_t arrayLayers, bool isCube = false)
{
	if (arrayLayers != filenames.size())
	{
		throw std::invalid_argument("Number of files must be equal to the array layer count");
	}

	device = pDevice->device;
	format = VK_FORMAT_R8G8B8A8_UNORM;

	// loads image bytes for each array layer
	std::vector<stbi_uc*> pixels(arrayLayers);
	for (int i = 0; i < arrayLayers; i++)
	{
		pixels[i] = loadPixels(filenames[i]);
	}

	// staging buffer to map its memory
	VkDeviceSize arrayLayerSize = extent.width * extent.height * STBI_rgb_alpha;
	StagingBuffer *pStagingBuffer = new StagingBuffer(pDevice, arrayLayerSize * arrayLayers);
	for (int i = 0; i < arrayLayers; i++)
	{
		pStagingBuffer->updateData(pixels[i], arrayLayerSize, i * arrayLayerSize);
		stbi_image_free(pixels[i]);
	}

	VkImageCreateFlags flags = 0;
	VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D;
	if (arrayLayers > 1)
	{
		if (isCube)
		{
			if (arrayLayers < 6)
			{
				throw std::invalid_argument("For cube texture number of array layers can't be less than six");
			}

			flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
			viewType = VK_IMAGE_VIEW_TYPE_CUBE;
		}
		else
		{
			flags = VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;
			viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
		}
	}

	mipLevels = static_cast<uint32_t>(std::ceil(
		std::log2(std::max(extent.width, extent.height)))
		);

	// texture image can't be mapped
	createThisImage(
		pDevice,
		extent,
		flags,
		mipLevels,
		format,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		arrayLayers
	);

	// before copying the layout of the texture image must be TRANSFER_DST
	VkImageSubresourceRange subresourceRange{
		VK_IMAGE_ASPECT_COLOR_BIT,
		0,
		mipLevels,
		0,
		arrayLayers
	};
	transitLayout(
		pDevice,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		subresourceRange
	);

	std::vector<VkBufferImageCopy> regions(arrayLayers);
	for (unsigned int i = 0; i < arrayLayers; i++)
	{
		regions[i] = VkBufferImageCopy{
			i * arrayLayerSize,					// bufferOffset;
			0,									// bufferRowLength;
			0,									// bufferImageHeight;
		{
			VK_IMAGE_ASPECT_COLOR_BIT,
			0,
			i,
			1
		},									// imageSubresource;
		{ 0, 0, 0 },						// imageOffset;
		{ extent.width, extent.height, 1 }	// imageExtent;
		};
	}
	pStagingBuffer->copyToImage(image, regions);
	delete(pStagingBuffer);

	// create other image objects
	generateMipmaps(pDevice, arrayLayers);
	createImageView(subresourceRange, viewType);
	createSampler();
}

TextureImage::~TextureImage()
{
	if (sampler != VK_NULL_HANDLE)
	{
		vkDestroySampler(device, sampler, nullptr);
	}
}

// protected:

stbi_uc* TextureImage::loadPixels(std::string filename)
{
	extent.depth = 1;
	stbi_uc *pixels = stbi_load(
		filename.c_str(),
		(int*)&extent.width,
		(int*)&extent.height,
		nullptr,
		STBI_rgb_alpha
	);
	if (!pixels)
	{
		LOGGER_FATAL(Logger::getTextureLoadingErrMsg(filename));
	}

	return pixels;
}

void TextureImage::generateMipmaps(Device *pDevice, uint32_t arrayLayers)
{
	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(pDevice->physicalDevice, format, &formatProperties);

	if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
	{
		LOGGER_FATAL(Logger::IMAGE_FORMAT_DOES_NOT_SUPPORT_LINEAR_BLITTING);
	}

	VkCommandBuffer commandBuffer = pDevice->beginOneTimeCommands();

	VkImageMemoryBarrier barrier{
		VK_STRUCTURE_TYPE_MEMORY_BARRIER,		// sType;
		nullptr,								// pNext;
		0,										// srcAccessMask;
		0,										// dstAccessMask;
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,	// oldLayout;
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,	// newLayout;
		VK_QUEUE_FAMILY_IGNORED,				// srcQueueFamilyIndex;
		VK_QUEUE_FAMILY_IGNORED,				// dstQueueFamilyIndex;
		image,									// image;
		{
			VK_IMAGE_ASPECT_COLOR_BIT,	// aspectMask
			0,							// baseMipLevel
			1,							// levelCount
			0,							// baseArrayLayer
			arrayLayers					// layerCount
		}										// subresourceRange;
	};

	int32_t mipWidth = extent.width;
	int32_t mipHeight = extent.height;

	for (int i = 1; i < mipLevels; i++)
	{   
		// transit current miplevel layout to TRANSFER_SRC
		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		vkCmdPipelineBarrier(
			commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		// scale and copy image form this to next miplevel
		VkImageBlit blit = {};
		// src area:
		blit.srcOffsets[0] = { 0, 0, 0 };
		blit.srcOffsets[1] = { 
			mipWidth, 
			mipHeight, 
			1 
		};
		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.mipLevel = i - 1;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = arrayLayers;
		// dst area:
		blit.dstOffsets[0] = { 0, 0, 0 };
		blit.dstOffsets[1] = { 
			mipWidth > 1 ? mipWidth / 2 : 1, 
			mipHeight > 1 ? mipHeight / 2 : 1, 
			1 
		};
		blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.mipLevel = i;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = arrayLayers;
		vkCmdBlitImage(
			commandBuffer,
			image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &blit,
			VK_FILTER_LINEAR
		);

		// transit current miplevel layout to SHADER_READ_ONLY
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		vkCmdPipelineBarrier(
			commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		// next miplevel scale
		if (mipWidth > 1) mipWidth /= 2;
		if (mipHeight > 1) mipHeight /= 2;
	}

	// transit last miplevel layout to SHADER
	barrier.subresourceRange.baseMipLevel = mipLevels - 1;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	vkCmdPipelineBarrier(
		commandBuffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);

	pDevice->endOneTimeCommands(commandBuffer);
}

void TextureImage::createSampler()
{
	VkSamplerCreateInfo createInfo{
		VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,	// sType;
		nullptr,								// pNext;
		0,										// flags;
		VK_FILTER_LINEAR,						// magFilter;
		VK_FILTER_LINEAR,						// minFilter;
		VK_SAMPLER_MIPMAP_MODE_LINEAR,			// mipmapMode;
		VK_SAMPLER_ADDRESS_MODE_REPEAT,			// addressModeU;
		VK_SAMPLER_ADDRESS_MODE_REPEAT,			// addressModeV;
		VK_SAMPLER_ADDRESS_MODE_REPEAT,			// addressModeW;
		0,										// mipLodBias;
		VK_TRUE,								// anisotropyEnable;
		16,										// maxAnisotropy;
		VK_FALSE,								// compareEnable;
		VK_COMPARE_OP_ALWAYS,					// compareOp;
		0,										// minLod;
		mipLevels,								// maxLod;
		VK_BORDER_COLOR_INT_OPAQUE_BLACK,		// borderColor;
		VK_FALSE,								// unnormalizedCoordinates;
	};

	VkResult result = vkCreateSampler(device, &createInfo, nullptr, &sampler);
	if (result != VK_SUCCESS)
	{
		LOGGER_FATAL(Logger::FAILED_TO_CREATE_TEXTURE_SAMPLER);
	}
}
