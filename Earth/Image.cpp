#include "Logger.h"

#include "Image.h"

// public:

Image::Image(
	Device *pDevice, 
	VkExtent3D extent, 
	uint32_t mipLevels, 
	VkFormat format, 
	VkImageTiling tiling, 
	VkImageUsageFlags usage, 
	VkMemoryPropertyFlags properties
)
{
	createThisImage(pDevice, extent, mipLevels, format, tiling, usage, properties);
}

Image::~Image()
{
	if (view != VK_NULL_HANDLE)
	{
		vkDestroyImageView(device, view, nullptr);
	}
	if (image != VK_NULL_HANDLE)
	{
		vkDestroyImage(device, image, nullptr);
	}
	if (memory != VK_NULL_HANDLE)
	{
		vkFreeMemory(device, memory, nullptr);
	}
}

void Image::createImageView(VkImageSubresourceRange subresourceRange)
{
	VkImageViewCreateInfo createInfo{
		VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,	// sType
		nullptr,									// pNext
		0,											// flags
		image,										// image
		VK_IMAGE_VIEW_TYPE_2D,						// viewType
		format,										// format
		VkComponentMapping(),						// components
		subresourceRange,							// subresourceRange
	};

	VkResult result = vkCreateImageView(device, &createInfo, nullptr, &view);
	if (result != VK_SUCCESS)
	{
		LOGGER_FATAL(Logger::FAILED_TO_CREATE_IMAGE_VIEW);
	}
}

void Image::transitLayout(Device *pDevice, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageSubresourceRange subresourceRange)
{
	VkCommandBuffer commandBuffer = pDevice->beginOneTimeCommands();

	VkImageMemoryBarrier barrier{
		VK_STRUCTURE_TYPE_MEMORY_BARRIER,	// sType;
		nullptr,							// pNext;
		0,									// srcAccessMask;
		0,									// dstAccessMask;
		oldLayout,							// oldLayout;
		newLayout,							// newLayout;
		VK_QUEUE_FAMILY_IGNORED,			// srcQueueFamilyIndex;
		VK_QUEUE_FAMILY_IGNORED,			// dstQueueFamilyIndex;
		image,								// image;
		subresourceRange,					// subresourceRange;
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
	else
	{
		LOGGER_FATAL(Logger::UNSUPPORTED_LAYOUT_TRANSITION);
	}

	vkCmdPipelineBarrier(
		commandBuffer,
		sourceStage, destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);

	pDevice->endOneTimeCommands(commandBuffer);
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
		&region
	);

	pDevice->endOneTimeCommands(commandBuffer);
}

// protected:

void Image::createThisImage(Device * pDevice, VkExtent3D extent, uint32_t mipLevels, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties)
{
	device = pDevice->device;
	this->extent = extent;
	this->format = format;

	VkImageCreateInfo imageInfo{
		VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,	// sType;
		nullptr,								// pNext;
		0,										// flags;
		VK_IMAGE_TYPE_2D,						// imageType;
		format,									// format;
		extent,									// extent;
		mipLevels,								// mipLevels;
		1,										// arrayLayers;
		VK_SAMPLE_COUNT_1_BIT,					// samples;
		tiling,									// tiling;
		usage,									// usage;
		VK_SHARING_MODE_EXCLUSIVE,				// sharingMode;
		0,										// queueFamilyIndexCount;
		nullptr,								// pQueueFamilyIndices;
		VK_IMAGE_LAYOUT_UNDEFINED				// initialLayout;
	};

	VkResult result = vkCreateImage(device, &imageInfo, nullptr, &image);
	if (result != VK_SUCCESS)
	{
		LOGGER_FATAL(Logger::FAILED_TO_CREATE_IMAGE);
	}

	allocateMemory(pDevice, properties);

	vkBindImageMemory(device, image, memory, 0);
}

// private:

void Image::allocateMemory(Device *pDevice, VkMemoryPropertyFlags properties)
{
	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(device, image, &memRequirements);

	uint32_t memoryTypeIndex = pDevice->findMemoryTypeIndex(
		memRequirements.memoryTypeBits,
		properties
	);

	VkMemoryAllocateInfo allocInfo =
	{
		VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,	// sType
		nullptr,								// pNext
		memRequirements.size,					// allocationSize
		memoryTypeIndex,						// memoryTypeIndex
	};

	VkResult result = vkAllocateMemory(device, &allocInfo, nullptr, &memory);
	if (result != VK_SUCCESS)
	{
		LOGGER_FATAL(Logger::FAILED_TO_ALLOC_IMAGE_MEMORY);
	}
}
