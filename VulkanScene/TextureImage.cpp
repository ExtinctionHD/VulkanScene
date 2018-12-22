#include <algorithm>
#include "StagingBuffer.h"
#define STB_IMAGE_IMPLEMENTATION

#include "TextureImage.h"

// public:

TextureImage::TextureImage(Device *pDevice, std::vector<std::string> filenames, uint32_t arrayLayers, bool isCube)
{
	assert(arrayLayers == filenames.size());

	// loads image bytes for each array layer
	std::vector<stbi_uc*> pixels(arrayLayers);
	for (uint32_t i = 0; i < arrayLayers; i++)
	{
		pixels[i] = loadPixels(filenames[i]);
	}

	this->pDevice = pDevice;
	format = VK_FORMAT_R8G8B8A8_UNORM;

	VkImageCreateFlags flags = 0;
	VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D;
	if (arrayLayers > 1)
	{
		if (isCube)
		{
			assert(arrayLayers >= 6);

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

	createThisImage(
		pDevice,
		extent,
		flags,
        VK_SAMPLE_COUNT_1_BIT,
		mipLevels,
		format,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		arrayLayers
	);

	updateData(reinterpret_cast<void**>(pixels.data()), STBI_rgb_alpha);

    // free image pixels
    for (auto arrayLayerPixels : pixels)
    {
		stbi_image_free(arrayLayerPixels);
    }

	// create other image objects
	generateMipmaps(pDevice, arrayLayers, VK_IMAGE_ASPECT_COLOR_BIT);
    
    VkImageSubresourceRange subresourceRange{
		VK_IMAGE_ASPECT_COLOR_BIT,
		0,
		mipLevels,
		0,
		arrayLayers
	};
	createImageView(subresourceRange, viewType);

	createSampler(VK_SAMPLER_ADDRESS_MODE_REPEAT);
}

TextureImage::TextureImage(
    Device *pDevice,
    VkExtent3D extent,
    VkImageCreateFlags flags,
	VkSampleCountFlagBits sampleCount,
    VkFormat format,
    VkImageTiling tiling,
    VkImageUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VkImageAspectFlags aspectFlags,
	VkImageViewType viewType,
    uint32_t arrayLayers,
    VkSamplerAddressMode samplerAddressMode
) : Image(pDevice, extent, flags, sampleCount, 1, format, tiling, usage | VK_IMAGE_USAGE_SAMPLED_BIT, properties, arrayLayers)
{
	VkImageSubresourceRange subresourceRange{
		aspectFlags,
		0,
		mipLevels,
		0,
		arrayLayers
	};

	createImageView(subresourceRange, viewType);

	createSampler(samplerAddressMode);
}

TextureImage::~TextureImage()
{
	if (sampler != VK_NULL_HANDLE)
	{
		vkDestroySampler(pDevice->device, sampler, nullptr);
	}
}

// protected:

stbi_uc* TextureImage::loadPixels(const std::string &filename)
{
	extent.depth = 1;
	stbi_uc *pixels = stbi_load(
		filename.c_str(),
		reinterpret_cast<int*>(&extent.width),
		reinterpret_cast<int*>(&extent.height),
		nullptr,
		STBI_rgb_alpha
	);

	assert(pixels);

	return pixels;
}

void TextureImage::generateMipmaps(Device *pDevice, uint32_t arrayLayers, VkImageAspectFlags aspectFlags)
{
	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(pDevice->physicalDevice, format, &formatProperties);

	assert(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT);

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
			aspectFlags,	            // aspectMask
			0,							// baseMipLevel
			1,							// levelCount
			0,							// baseArrayLayer
			arrayLayers					// layerCount
		}										// subresourceRange;
	};

	int32_t mipWidth = extent.width;
	int32_t mipHeight = extent.height;

	for (uint32_t i = 1; i < mipLevels; i++)
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
		blit.srcSubresource.aspectMask = aspectFlags;
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
		blit.dstSubresource.aspectMask = aspectFlags;
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

	// transit last miplevel layout to SHADER_READ_ONLY_OPTIMAL
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

void TextureImage::createSampler(VkSamplerAddressMode addressMode)
{
	VkSamplerCreateInfo createInfo{
		VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,	// sType;
		nullptr,								// pNext;
		0,										// flags;
		VK_FILTER_LINEAR,						// magFilter;
		VK_FILTER_LINEAR,						// minFilter;
		VK_SAMPLER_MIPMAP_MODE_LINEAR,			// mipmapMode;
		addressMode,			                // addressModeU;
		addressMode,			                // addressModeV;
		addressMode,			                // addressModeW;
		0,										// mipLodBias;
		VK_TRUE,								// anisotropyEnable;
		4.0f,									// maxAnisotropy;
		VK_FALSE,								// compareEnable;
		VK_COMPARE_OP_ALWAYS,					// compareOp;
		0,										// minLod;
		float(mipLevels),						// maxLod;
		VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,		// borderColor;
		VK_FALSE,								// unnormalizedCoordinates;
	};

	VkResult result = vkCreateSampler(pDevice->device, &createInfo, nullptr, &sampler);
	assert(result == VK_SUCCESS);
}
