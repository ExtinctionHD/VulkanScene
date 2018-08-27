#pragma once

#include "Device.h"
#include "SwapChainImage.h"

// allocates memory and creates new image,
// can create image view, and transit its layout
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

	VkExtent3D extent;

	VkImageView view = VK_NULL_HANDLE;

	void createImageView(VkImageSubresourceRange subresourceRange);

	void transitLayout(Device *pDevice, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageSubresourceRange subresourceRange);


private:
	VkDeviceMemory memory = VK_NULL_HANDLE;

	void allocateMemory(Device *pDevice, VkMemoryPropertyFlags properties);
};

