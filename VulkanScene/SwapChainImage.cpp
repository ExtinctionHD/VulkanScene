#include <iostream>
#include <cassert>

#include "SwapChainImage.h"

SwapChainImage::SwapChainImage(Device *pDevice, VkImage image, VkFormat format)
{
	this->image = image;
	this->format = format;
	this->pDevice = pDevice;
}

VkImageView SwapChainImage::getImageView(VkImageSubresourceRange subresourceRange) const
{
	VkImageView imageView;

	VkImageViewCreateInfo createInfo{
		VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,	// sType
		nullptr,									// pNext
		0,											// flags
		image,										// image
		VK_IMAGE_VIEW_TYPE_2D,						// viewType
		format,										// format
		VkComponentMapping(),						// components
		subresourceRange,							// subresourceRange
	};

	VkResult result = vkCreateImageView(pDevice->getVk(), &createInfo, nullptr, &imageView);
	assert(result == VK_SUCCESS);

	return imageView;
}

VkFormat SwapChainImage::getFormat() const
{
	return format;
}
