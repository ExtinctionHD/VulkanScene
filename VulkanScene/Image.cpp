#include <cassert>
#include "StagingBuffer.h"

#include "Image.h"

// public:

Image::Image(
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
	VkImageAspectFlags aspectFlags)
{
	createThisImage(
		device,
		extent,
		flags,
		sampleCount,
		mipLevels,
		format,
		tiling,
		usage,
		arrayLayers,
		cubeMap,
		properties,
		aspectFlags);
}

Image::~Image()
{
	vkDestroyImageView(device->get(), view, nullptr);
	vkDestroyImage(device->get(), image, nullptr);
	vkFreeMemory(device->get(), memory, nullptr);
}

VkSampleCountFlagBits Image::getSampleCount() const
{
	return sampleCount;
}

void Image::transitLayout(
    VkImageLayout oldLayout,
    VkImageLayout newLayout,
    VkImageSubresourceRange subresourceRange) const
{
	VkCommandBuffer commandBuffer = device->beginOneTimeCommands();

	VkImageMemoryBarrier barrier{
		VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		nullptr,								
		0,										
		0,										
		oldLayout,								
		newLayout,								
		VK_QUEUE_FAMILY_IGNORED,				
		VK_QUEUE_FAMILY_IGNORED,				
		image,									
		subresourceRange,						
	};

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	}
	else
	{
		throw std::invalid_argument("Unsupported image layout transition");
	}

	vkCmdPipelineBarrier(
		commandBuffer,
		sourceStage, destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier);

	device->endOneTimeCommands(commandBuffer);
}

void Image::updateData(std::vector<const void*> data, uint32_t layersOffset, uint32_t pixelSize) const
{
	assert(layersOffset + data.size() <= arrayLayers);

	const uint32_t updatedLayers = arrayLayers < layersOffset + uint32_t(data.size())
                                       ? arrayLayers - layersOffset
                                       : uint32_t(data.size());

	const VkImageSubresourceRange subresourceRange{
		VK_IMAGE_ASPECT_COLOR_BIT,
		0,
		mipLevels,
		layersOffset,
		updatedLayers
	};
	const VkDeviceSize layerSize = extent.width * extent.height * pixelSize;

	StagingBuffer stagingBuffer(device, layerSize * updatedLayers);
	for (uint32_t i = 0; i < updatedLayers; i++)
	{
		stagingBuffer.updateData(data[i], layerSize, i * layerSize);
	}

	std::vector<VkBufferImageCopy> regions(updatedLayers);
	for (uint32_t i = 0; i < updatedLayers; i++)
	{
		regions[i] = VkBufferImageCopy{
			i * layerSize,
			0,
			0,
			{
				VK_IMAGE_ASPECT_COLOR_BIT,
				0,
				layersOffset + i,
				1
			},
			{ 0, 0, 0 },
			{ extent.width, extent.height, 1 }
		};
	}

	// before copying the layout of the image must be TRANSFER_DST
	transitLayout(
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		subresourceRange);

	stagingBuffer.copyToImage(image, regions);
}

void Image::copyImage(Device *device, Image &srcImage, Image &dstImage, VkExtent3D extent, VkImageSubresourceLayers subresourceLayers)
{
	VkCommandBuffer commandBuffer = device->beginOneTimeCommands();

	VkImageCopy region{
		subresourceLayers,	
		{ 0, 0, 0},
		subresourceLayers,
		{ 0, 0, 0 },
		extent	
	};

	vkCmdCopyImage(
		commandBuffer,
		srcImage.image,
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		dstImage.image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&region);

	device->endOneTimeCommands(commandBuffer);
}

// protected:

void Image::createThisImage(
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
	VkImageAspectFlags aspectFlags)
{
	this->device = device;
	this->extent = extent;
	this->format = format;
	this->sampleCount = sampleCount;
	this->mipLevels = mipLevels;
	this->arrayLayers = arrayLayers;

	VkImageType imageType = VK_IMAGE_TYPE_1D;
	VkImageViewType viewType = arrayLayers == 1 ? VK_IMAGE_VIEW_TYPE_1D : VK_IMAGE_VIEW_TYPE_1D_ARRAY;
	if (extent.height > 0)
	{
		imageType = VK_IMAGE_TYPE_2D;
		viewType = VK_IMAGE_VIEW_TYPE_2D;

		if (cubeMap)
		{
			assert(arrayLayers >= 6);

			flags = flags | VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
			viewType = VK_IMAGE_VIEW_TYPE_CUBE;
		}
		else if (arrayLayers > 1)
		{
			flags = VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;
			viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
		}
	}
	else
	{
		imageType = VK_IMAGE_TYPE_1D;
	}

	VkImageCreateInfo imageInfo{
		VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		nullptr,
		flags,
		imageType,
		format,
		extent,
		mipLevels,
		arrayLayers,
		sampleCount,
		tiling,
		usage,
		VK_SHARING_MODE_EXCLUSIVE,
		0,
		nullptr,
		VK_IMAGE_LAYOUT_UNDEFINED
	};

	const VkResult result = vkCreateImage(device->get(), &imageInfo, nullptr, &image);
	assert(result == VK_SUCCESS);

	allocateMemory(device, properties);

	vkBindImageMemory(device->get(), image, memory, 0);

	const VkImageSubresourceRange subresourceRange{
		aspectFlags,
		0,
		mipLevels,
		0,
		arrayLayers
	};
	createImageView(subresourceRange, viewType);
}

// private:

void Image::allocateMemory(Device *device, VkMemoryPropertyFlags properties)
{
	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(device->get(), image, &memRequirements);

    const uint32_t memoryTypeIndex = device->findMemoryTypeIndex(
		memRequirements.memoryTypeBits,
		properties);

	VkMemoryAllocateInfo allocInfo{
		VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		nullptr,			
		memRequirements.size,	
		memoryTypeIndex,		
	};

    const VkResult result = vkAllocateMemory(device->get(), &allocInfo, nullptr, &memory);
	assert(result == VK_SUCCESS);
}
