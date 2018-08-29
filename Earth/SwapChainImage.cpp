#include "SwapChainImage.h"

#include "Logger.h"

SwapChainImage::SwapChainImage(VkDevice device, VkImage image, VkFormat format)
{
	this->image = image;
	this->format = format;
	this->device = device;
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

	VkResult result = vkCreateImageView(device, &createInfo, nullptr, &imageView);
	if (result != VK_SUCCESS)
	{
		LOGGER_FATAL(Logger::FAILED_TO_CREATE_IMAGE_VIEW);
	}

	return imageView;
}
