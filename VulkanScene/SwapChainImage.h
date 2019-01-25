#pragma once

#include <vulkan/vulkan.h>
#include "Device.h"

// class that help create view of swapchain images
class SwapChainImage
{
public:
	SwapChainImage() = default;

	virtual ~SwapChainImage() = default;

	SwapChainImage(Device *device, VkImage image, VkFormat format, VkImageSubresourceRange subresourceRange);

	VkImage get() const;

	VkImageView getView() const;

	VkImageView createImageView(VkImageSubresourceRange subresourceRange, VkImageViewType viewType) const;

	VkFormat getFormat() const;

protected:
	Device *device;

	VkImage image;

	VkImageView view;

	VkFormat format;
};

