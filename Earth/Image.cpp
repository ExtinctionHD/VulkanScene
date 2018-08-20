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
	device = pDevice->device;
	this->format = format;

	VkImageCreateInfo imageInfo = 
	{
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

Image::~Image()
{
	vkDestroyImageView(device, view, nullptr);
	vkDestroyImage(device, image, nullptr);
	vkFreeMemory(device, memory, nullptr);
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
