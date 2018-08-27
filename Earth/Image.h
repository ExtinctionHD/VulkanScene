#pragma once

#include "Device.h"
#include "SwapChainImage.h"

// allocate memory and create new image
class Image: public SwapChainImage
{
public:
	Image() {}

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

	VkImageView view = VK_NULL_HANDLE;

	void createImageView(VkImageAspectFlags aspectFlags, uint32_t mipLevels);

	void transitLayout(Device *pDevice, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageSubresourceRange subresourceRange);


private:
	VkDeviceMemory memory = VK_NULL_HANDLE;

	void allocateMemory(Device *pDevice, VkMemoryPropertyFlags properties);
};

