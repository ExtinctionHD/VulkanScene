#include <iostream>
#include <cassert>

#include "SwapChainImage.h"

// public:

SwapChainImage::SwapChainImage(Device *device, VkImage image, VkFormat format, VkImageSubresourceRange subresourceRange)
{
	this->device = device;
	this->image = image;
	this->format = format;

	createImageView(subresourceRange, VK_IMAGE_VIEW_TYPE_2D);
}

VkImage SwapChainImage::get() const
{
	return image;
}

VkImageView SwapChainImage::getView() const
{
	return view;
}

VkFormat SwapChainImage::getFormat() const
{
	return format;
}

// private:

void SwapChainImage::createImageView(VkImageSubresourceRange subresourceRange, VkImageViewType viewType)
{
	VkImageViewCreateInfo createInfo{
		VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		nullptr,
		0,
		image,
		viewType,
		format,
		VkComponentMapping(),
		subresourceRange,
	};

	const VkResult result = vkCreateImageView(device->get(), &createInfo, nullptr, &view);
	assert(result == VK_SUCCESS);
}
