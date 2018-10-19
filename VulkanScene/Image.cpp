#include <cassert>

#include "Image.h"

// public:

Image::Image(
	Device *pDevice, 
	VkExtent3D extent, 
	VkImageCreateFlags flags,
	uint32_t mipLevels, 
	VkFormat format, 
	VkImageTiling tiling, 
	VkImageUsageFlags usage, 
	VkMemoryPropertyFlags properties,
	uint32_t arrayLayers
)
{
	createThisImage(pDevice, extent, flags, mipLevels, format, tiling, usage, properties, arrayLayers);
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
	if (stagingMemory != VK_NULL_HANDLE)
	{
		vkFreeMemory(device, stagingMemory, nullptr);
	}
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

	VkResult result = vkCreateImageView(device, &createInfo, nullptr, &view);
	assert(result == VK_SUCCESS);
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
		throw std::invalid_argument("Unsupported image layout transition");
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

void Image::updateData(uint8_t *pixels, size_t pixelSize, uint32_t arrayLayer)
{
	void *data;
	VkDeviceSize size = extent.width * extent.height * pixelSize;
	vkMapMemory(device, stagingMemory, 0, size, 0, &data);

	VkImageSubresource subresource{
		VK_IMAGE_ASPECT_COLOR_BIT,
		0,
		arrayLayer
	};

	VkSubresourceLayout layout;
	vkGetImageSubresourceLayout(
		device,
		image,
		&subresource,
		&layout
	);

	if (layout.rowPitch == extent.width * pixelSize)
	{
		memcpy(data, pixels, size);
	}
	else
	{
		uint8_t *dataBytes = reinterpret_cast<uint8_t*>(data);

		for (int y = 0; y < extent.height; y++)
		{
			memcpy(
				&dataBytes[y * layout.rowPitch],
				&pixels[y * extent.width * pixelSize],
				extent.width * pixelSize
			);
		}
	}

	vkUnmapMemory(device, stagingMemory);
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

void Image::createThisImage(
	Device *pDevice, 
	VkExtent3D extent, 
	VkImageCreateFlags flags, 
	uint32_t mipLevels, 
	VkFormat format, 
	VkImageTiling tiling, 
	VkImageUsageFlags usage, 
	VkMemoryPropertyFlags properties, 
	uint32_t arrayLayers
)
{
	device = pDevice->device;
	this->extent = extent;
	this->format = format;

	VkImageCreateInfo imageInfo{
		VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,	// sType;
		nullptr,								// pNext;
		flags,									// flags;
		VK_IMAGE_TYPE_2D,						// imageType;
		format,									// format;
		extent,									// extent;
		mipLevels,								// mipLevels;
		arrayLayers,							// arrayLayers;
		VK_SAMPLE_COUNT_1_BIT,					// samples;
		tiling,									// tiling;
		usage,									// usage;
		VK_SHARING_MODE_EXCLUSIVE,				// sharingMode;
		0,										// queueFamilyIndexCount;
		nullptr,								// pQueueFamilyIndices;
		VK_IMAGE_LAYOUT_UNDEFINED				// initialLayout;
	};
	if (extent.depth > 1)
	{
		imageInfo.imageType = VK_IMAGE_TYPE_3D;
	}

	VkResult result = vkCreateImage(device, &imageInfo, nullptr, &image);
	assert(result == VK_SUCCESS);

	allocateMemory(pDevice, properties);

	vkBindImageMemory(device, image, stagingMemory, 0);
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

	VkResult result = vkAllocateMemory(device, &allocInfo, nullptr, &stagingMemory);
	assert(result == VK_SUCCESS);
}
