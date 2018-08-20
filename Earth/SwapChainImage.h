#pragma once

#include <vulkan/vulkan.h>

// class that help create view of swapchain images
class SwapChainImage
{
public:
	VkImage image;

	VkFormat format;

	VkImageView view;

	SwapChainImage() {}

	// save device, image, format
	SwapChainImage(VkDevice device, VkImage image, VkFormat format);

	// create view of saved image
	void createImageView(VkImageAspectFlags aspectFlags, uint32_t mipLevels);

protected:
	VkDevice device;  // device that control this image
};

