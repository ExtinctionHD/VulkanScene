#include <cassert>
#include "StagingBuffer.h"

#include "Image.h"

// public:

Image::Image(
	Device *pDevice, 
	VkExtent3D extent, 
	VkImageCreateFlags flags,
	VkSampleCountFlagBits sampleCount,
	uint32_t mipLevels, 
	VkFormat format, 
	VkImageTiling tiling, 
	VkImageUsageFlags usage, 
	VkMemoryPropertyFlags properties,
	uint32_t arrayLayers)
{
	createThisImage(pDevice, extent, flags, sampleCount, mipLevels, format, tiling, usage, properties, arrayLayers);
}

Image::~Image()
{
	if (view != nullptr)
	{
		vkDestroyImageView(pDevice->getVk(), view, nullptr);
		view = nullptr;
	}
	if (image != nullptr)
	{
		vkDestroyImage(pDevice->getVk(), image, nullptr);
		view = nullptr;
	}
	if (stagingMemory != nullptr)
	{
		vkFreeMemory(pDevice->getVk(), stagingMemory, nullptr);
		view = nullptr;
	}
}

VkSampleCountFlagBits Image::getSampleCount() const
{
	return sampleCount;
}

void Image::createImageView(VkImageSubresourceRange subresourceRange, VkImageViewType viewType)
{
	VkImageViewCreateInfo createInfo{
		VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,	// sType
		nullptr,									// pNext
		0,											// flags
		image,										// image
		viewType,									// viewType
		format,										// format
		VkComponentMapping(),						// components
		subresourceRange,							// subresourceRange
	};

	VkResult result = vkCreateImageView(pDevice->getVk(), &createInfo, nullptr, &view);
	assert(result == VK_SUCCESS);
}

void Image::transitLayout(Device *pDevice, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageSubresourceRange subresourceRange)
{
	VkCommandBuffer commandBuffer = pDevice->beginOneTimeCommands();

	VkImageMemoryBarrier barrier{
		VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,	// sType;
		nullptr,								// pNext;
		0,										// srcAccessMask;
		0,										// dstAccessMask;
		oldLayout,								// oldLayout;
		newLayout,								// newLayout;
		VK_QUEUE_FAMILY_IGNORED,				// srcQueueFamilyIndex;
		VK_QUEUE_FAMILY_IGNORED,				// dstQueueFamilyIndex;
		image,									// image;
		subresourceRange,						// subresourceRange;
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

	pDevice->endOneTimeCommands(commandBuffer);
}

void Image::updateData(void **data, uint32_t pixelSize)
{
	VkImageSubresourceRange subresourceRange{
		VK_IMAGE_ASPECT_COLOR_BIT,
		0,
		mipLevels,
		0,
		arrayLayers
	};

	// before copying the layout of the texture image must be TRANSFER_DST
	transitLayout(
		pDevice,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		subresourceRange);

	// staging buffer to map its memory
	VkDeviceSize arrayLayerSize = extent.width * extent.height * pixelSize;
	StagingBuffer *pStagingBuffer = new StagingBuffer(pDevice, arrayLayerSize * arrayLayers);
	for (uint32_t i = 0; i < arrayLayers; i++)
	{
		pStagingBuffer->updateData(data[i], arrayLayerSize, i * arrayLayerSize);
	}

	std::vector<VkBufferImageCopy> regions(arrayLayers);
	for (uint32_t i = 0; i < arrayLayers; i++)
	{
		regions[i] = VkBufferImageCopy{
			i * arrayLayerSize,
			0,
			0,
		    {
			    VK_IMAGE_ASPECT_COLOR_BIT,
			    0,
			    i,
			    1
		    },
		    { 0, 0, 0 },	
		    { extent.width, extent.height, 1 }
		};
	}
	pStagingBuffer->copyToImage(image, regions);
	delete pStagingBuffer;
}

void Image::copyImage(Device *pDevice, Image& srcImage, Image& dstImage, VkExtent3D extent, VkImageSubresourceLayers subresourceLayers)
{
	VkCommandBuffer commandBuffer = pDevice->beginOneTimeCommands();

	VkImageCopy region{
		subresourceLayers,	// srcSubresource;
		{ 0, 0, 0},			// srcOffset;
		subresourceLayers,	// dstSubresource;
		{ 0, 0, 0 },		// dstOffset;
		extent				// extent;
	};

	vkCmdCopyImage(
		commandBuffer,
		srcImage.image,
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		dstImage.image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&region);

	pDevice->endOneTimeCommands(commandBuffer);
}

// protected:

void Image::createThisImage(
	Device *pDevice, 
	VkExtent3D extent, 
	VkImageCreateFlags flags,
	VkSampleCountFlagBits sampleCount,
	uint32_t mipLevels, 
	VkFormat format, 
	VkImageTiling tiling, 
	VkImageUsageFlags usage, 
	VkMemoryPropertyFlags properties, 
	uint32_t arrayLayers)
{
	this->pDevice = pDevice;
	this->extent = extent;
	this->format = format;
	this->sampleCount = sampleCount;
	this->mipLevels = mipLevels;
	this->arrayLayers = arrayLayers;

	VkImageCreateInfo imageInfo{
		VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		nullptr,
		flags,
		VK_IMAGE_TYPE_2D,
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
	if (extent.depth > 1)
	{
		imageInfo.imageType = VK_IMAGE_TYPE_3D;
	}

	VkResult result = vkCreateImage(pDevice->getVk(), &imageInfo, nullptr, &image);
	assert(result == VK_SUCCESS);

	allocateMemory(pDevice, properties);

	vkBindImageMemory(pDevice->getVk(), image, stagingMemory, 0);
}

// private:

void Image::allocateMemory(Device *pDevice, VkMemoryPropertyFlags properties)
{
	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(pDevice->getVk(), image, &memRequirements);

	uint32_t memoryTypeIndex = pDevice->findMemoryTypeIndex(
		memRequirements.memoryTypeBits,
		properties);

	VkMemoryAllocateInfo allocInfo =
	{
		VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,	// sType
		nullptr,								// pNext
		memRequirements.size,					// allocationSize
		memoryTypeIndex,						// memoryTypeIndex
	};

	VkResult result = vkAllocateMemory(pDevice->getVk(), &allocInfo, nullptr, &stagingMemory);
	assert(result == VK_SUCCESS);
}
