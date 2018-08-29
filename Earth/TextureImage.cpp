#include "Logger.h"
#include <algorithm>
#define STB_IMAGE_IMPLEMENTATION

#include "TextureImage.h"

// public:

TextureImage::TextureImage(Device *pDevice, std::string filename)
{
	device = pDevice->device;
	format = VK_FORMAT_R8G8B8A8_UNORM;

	// image bytes
	stbi_uc *pixels = loadPixels(filename);

	// staging image can be mapped
	Image *pStagingImage = new Image(
		pDevice,
		extent,
		1,
		format,
		VK_IMAGE_TILING_LINEAR,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
	);
	pStagingImage->updateData(pixels, STBI_rgb_alpha);

	stbi_image_free(pixels);

	// texture image can't be mapped
	mipLevels = static_cast<uint32_t>(std::ceil(
		std::log2(std::max(extent.width, extent.height)))
	);
	createThisImage(
		pDevice,
		extent,
		mipLevels,
		VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	);

	// before copying the layout of the staging image must be TRANSFER_SRC
	VkImageSubresourceRange subresourceRange{
		VK_IMAGE_ASPECT_COLOR_BIT,
		0,
		1,
		0,
		1
	};
	pStagingImage->transitLayout(
		pDevice,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		subresourceRange
	);

	// and texture image layout - TRANSFER_DST
	subresourceRange.levelCount = mipLevels;
	transitLayout(
		pDevice,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		subresourceRange
	);

	// copy staging image to texture
	VkImageSubresourceLayers subresourceLayers{
		VK_IMAGE_ASPECT_COLOR_BIT,	// aspectMask;
		0,							// mipLevel;
		0,							// baseArrayLayer;
		1,							// layerCount;
	};
	Image::copyImage(pDevice, *pStagingImage, *this, extent, subresourceLayers);

	delete(pStagingImage);

	// create other image objects
	generateMipmaps(pDevice);
	createImageView(subresourceRange);
	createSampler();
}

TextureImage::~TextureImage()
{
	if (sampler != VK_NULL_HANDLE)
	{
		vkDestroySampler(device, sampler, nullptr);
	}
}

// private:

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

void TextureImage::generateMipmaps(Device *pDevice)
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
			1							// layerCount
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
		blit.srcSubresource.layerCount = 1;
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
		blit.dstSubresource.layerCount = 1;
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
