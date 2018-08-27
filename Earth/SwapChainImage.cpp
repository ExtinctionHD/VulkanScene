#include "SwapChainImage.h"

#include "Logger.h"

SwapChainImage::SwapChainImage(VkDevice device, VkImage image, VkFormat format)
{
	this->image = image;
	this->format = format;
	this->device = device;
}

VkImageView SwapChainImage::createImageView(VkImageAspectFlags aspectFlags, uint32_t mipLevels)
{
	VkImageView imageView;

	VkImageSubresourceRange subresourceRange{
		aspectFlags,	// aspectMask;
		0,				// baseMipLevel;
		mipLevels,		// levelCount;
		0,				// baseArrayLayer;
		1,				// layerCount;
	};

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
