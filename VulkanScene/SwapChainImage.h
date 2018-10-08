#pragma once

#include <vulkan/vulkan.h>

// class that help create view of swapchain images
class SwapChainImage
{
public:
	VkImage image = VK_NULL_HANDLE;

	VkFormat format;

	SwapChainImage() {}

	// save device, image, format
	SwapChainImage(VkDevice device, VkImage image, VkFormat format);

	// create view of saved image
	VkImageView getImageView(VkImageSubresourceRange subresourceRange) const;

protected:
	// device that provides this buffer and memory
	VkDevice device;
};

