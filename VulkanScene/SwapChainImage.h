#pragma once

#include <vulkan/vulkan.h>
#include "Device.h"

// class that help create view of swapchain images
class SwapChainImage
{
public:
	SwapChainImage() = default;

    // save device, image, format
	SwapChainImage(Device *pDevice, VkImage image, VkFormat format);

	VkFormat format{};

	// create view of saved image
	VkImageView getImageView(VkImageSubresourceRange subresourceRange) const;

	VkFormat getFormat() const;

protected:
	// device that provides this buffer and memory
	Device *pDevice;

	VkImage image = VK_NULL_HANDLE;
};

