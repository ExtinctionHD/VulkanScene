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

	VkImageView view;

	void createImageView(VkImageAspectFlags aspectFlags, uint32_t mipLevels);

	void transitionImageLayout(Device *pDevice, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageSubresourceRange subresourceRange);


private:
	VkDeviceMemory memory;

	void allocateMemory(Device *pDevice, VkMemoryPropertyFlags properties);
};

