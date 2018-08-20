#pragma once

#include "Device.h"
#include "SwapChainImage.h"

// allocate memory and create new image
class Image: public SwapChainImage
{
public:
	Image(
		Device *pDevice,
		VkExtent3D extent,
		uint32_t mipLevels,
		VkFormat format,
		VkImageTiling tiling,
		VkImageUsageFlags usage,
		VkMemoryPropertyFlags properties
	);

	~Image();

private:
	VkDeviceMemory memory;

	void allocateMemory(Device *pDevice, VkMemoryPropertyFlags properties);
};

