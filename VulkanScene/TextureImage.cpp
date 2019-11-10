#include <algorithm>
#include "StagingBuffer.h"
#define STB_IMAGE_IMPLEMENTATION

#include "TextureImage.h"
#include "File.h"
#include <cmath>

// public:

TextureImage::TextureImage(
	Device *device,
	const std::vector<std::string> &paths,
	uint32_t arrayLayers,
	bool cubeMap,
	VkFilter filter,
	VkSamplerAddressMode samplerAddressMode)
{
	assert(arrayLayers == paths.size());

	// loads image bytes for each array layer
	std::vector<const void*> pixels(arrayLayers);
	for (uint32_t i = 0; i < arrayLayers; i++)
	{
		pixels[i] = loadPixels(paths[i]);
	}

	this->device = device;

	format = VK_FORMAT_R8G8B8A8_UNORM;
	extent.depth = 1;
	mipLevels = static_cast<uint32_t>(std::ceil(
		std::log2(std::max(extent.width, extent.height))));
	mipLevels = mipLevels > 0 ? mipLevels : 1;

	createThisImage(
		device,
		extent,
		0,
		VK_SAMPLE_COUNT_1_BIT,
		mipLevels,
		format,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		arrayLayers,
		cubeMap,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		VK_IMAGE_ASPECT_COLOR_BIT);


	updateData(pixels, 0, STBI_rgb_alpha);

    for (auto arrayLayerPixels : pixels)
    {
		stbi_image_free(const_cast<void*>(arrayLayerPixels));
    }

	// create other image objects
	generateMipmaps(device, VK_IMAGE_ASPECT_COLOR_BIT, filter);

	createSampler(filter, samplerAddressMode);
}

TextureImage::TextureImage(
	Device *device,
	VkExtent3D extent,
	VkImageCreateFlags flags,
	VkSampleCountFlagBits sampleCount,
	uint32_t mipLevels,
	VkFormat format,
	VkImageTiling tiling,
	VkImageUsageFlags usage,
	uint32_t arrayLayers,
	bool cubeMap,
	VkMemoryPropertyFlags properties,
	VkImageAspectFlags aspectFlags,
	VkFilter filter,
	VkSamplerAddressMode samplerAddressMode)
	: Image(
		device,
		extent,
		flags,
		sampleCount,
		mipLevels,
		format,
		tiling,
		usage | VK_IMAGE_USAGE_SAMPLED_BIT,
		arrayLayers,
		cubeMap,
		properties,
		aspectFlags)
{
	createSampler(filter, samplerAddressMode);
}

TextureImage::~TextureImage()
{
	vkDestroySampler(device->get(), sampler, nullptr);
}

VkSampler TextureImage::getSampler() const
{
	return sampler;
}

// protected:

stbi_uc* TextureImage::loadPixels(const std::string &path)
{
	stbi_uc *pixels = stbi_load(
		File::getAbsolute(path).c_str(),
		reinterpret_cast<int*>(&extent.width),
		reinterpret_cast<int*>(&extent.height),
		nullptr,
		STBI_rgb_alpha);

	assert(pixels);

	return pixels;
}

void TextureImage::generateMipmaps(
    Device *device,
    VkImageAspectFlags aspectFlags,
    VkFilter filter) const
{
	const auto featureFlags = device->getFormatProperties(format).optimalTilingFeatures;
	assert(featureFlags & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT);

	VkCommandBuffer commandBuffer = device->beginOneTimeCommands();

	VkImageMemoryBarrier barrier{
		VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		nullptr,								
		0,										
		0,										
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,	
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,	
		VK_QUEUE_FAMILY_IGNORED,				
		VK_QUEUE_FAMILY_IGNORED,				
		image,									
		{
			aspectFlags,	
			0,			
			1,						
			0,						
			arrayLayers				
		}										
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
			1, &barrier);

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
			filter);

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
			1, &barrier);

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
		1, &barrier);

	device->endOneTimeCommands(commandBuffer);
}

void TextureImage::createSampler(VkFilter filter, VkSamplerAddressMode addressMode)
{
	VkSamplerCreateInfo createInfo{
		VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
		nullptr,								
		0,										
		filter,									
		filter,									
		VK_SAMPLER_MIPMAP_MODE_LINEAR,			
		addressMode,			              
		addressMode,			              
		addressMode,			              
		0,										
		true,								
		16.0f,									
		false,								
		VK_COMPARE_OP_ALWAYS,					
		0,										
		float(mipLevels),						
		VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,		
		false,								
	};
	if (filter == VK_FILTER_NEAREST)
	{
		createInfo.anisotropyEnable = false;
		createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
	}

    const VkResult result = vkCreateSampler(device->get(), &createInfo, nullptr, &sampler);
	assert(result == VK_SUCCESS);
}
