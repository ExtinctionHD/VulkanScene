#include <algorithm>
#include "SurfaceSupportDetails.h"
#include "Device.h"
#include "Logger.h"
#include "SwapChainImage.h"

#include "SwapChain.h"

// public:

SwapChain::SwapChain(Device *pDevice, VkSurfaceKHR surface, VkExtent2D surfaceExtent)
{
	device = pDevice->device;

	// get necessary swapchain properties
	SurfaceSupportDetails details = pDevice->getSurfaceSupportDetails();
	VkSurfaceFormatKHR surfaceFormat = chooseSurfaceFormat(details.formats);
	VkPresentModeKHR presentMode = choosePresentMode(details.presentModes);
	extent = chooseExtent(details.capabilities, surfaceExtent);
	imageCount = chooseImageCount(details.capabilities);
	imageFormat = surfaceFormat.format;

	VkSwapchainCreateInfoKHR createInfo = 
	{
		VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,	// sType
		nullptr,										// pNext
		0,										        // flags
		surface,										// surface
		imageCount,										// minImageCount
		surfaceFormat.format,							// imageFormat
		surfaceFormat.colorSpace,						// imageColorSpace
		extent,											// imageExtent
		1,												// imageArrayLayers
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,			// imageUsage
		VK_SHARING_MODE_EXCLUSIVE,						// imageSharingMode
		0,												// queueFamilyIndexCount
		nullptr,										// pQueueFamilyIndices
		details.capabilities.currentTransform,			// preTransform
		VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,				// compositeAlpha
		presentMode,									// presentMode
		VK_TRUE,										// clipped
		nullptr,										// oldSwapchain
	};

	// concurrent sharing mode only when using different queue families
	std::vector<uint32_t> indices =
	{
		(uint32_t)pDevice->getQueueFamilyIndices().graphics,
		(uint32_t)pDevice->getQueueFamilyIndices().present
	};
	if (indices[0] != indices[1])
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = indices.size();
		createInfo.pQueueFamilyIndices = indices.data();
	}

	VkResult result = vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain);
	if (result != VK_SUCCESS)
	{
		LOGGER_FATAL(Logger::FAILED_TO_CREATE_SWAPCHAIN);
	}

	// save swapchain images 
	initImages();

	// create to each image imageView
	createImageViews();
}

SwapChain::~SwapChain()
{
	for (int i = 0; i < imageViews.size(); i++)
	{
		vkDestroyImageView(device, imageViews[i], nullptr);
	}

	vkDestroySwapchainKHR(device, swapChain, nullptr);
}

// private:

VkSurfaceFormatKHR SwapChain::chooseSurfaceFormat(std::vector<VkSurfaceFormatKHR> availableFormats) const
{
	// can choose any format
	if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED)
	{
		return PRESENT_FORMAT;
	}

	// try to found preferred format from available
	for (const auto& availableFormat : availableFormats)
	{
		if (availableFormat.format == PRESENT_FORMAT.format &&
			availableFormat.colorSpace == PRESENT_FORMAT.colorSpace)
		{
			return availableFormat;
		}
	}

	// first available format
	return availableFormats[0];
}

VkPresentModeKHR SwapChain::choosePresentMode(std::vector<VkPresentModeKHR> availablePresentModes) const
{
	for (const auto& availablePresentMode : availablePresentModes)
	{
		if (availablePresentMode == PREFERRED_PRESENT_MODE)
		{
			return availablePresentMode;
		}
	}

	// simplest mode
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D SwapChain::chooseExtent(VkSurfaceCapabilitiesKHR capabilities, VkExtent2D actualExtent)
{
	// can choose any extent
	if (capabilities.currentExtent.width != (std::numeric_limits<uint32_t>::max)())
	{
		return capabilities.currentExtent;
	}
	else
	{
		// extent height and width: (minAvailable <= extent <= maxAvailable) && (extent <= actualExtent)
		actualExtent.width = (std::max)(
			capabilities.minImageExtent.width,
			(std::min)(capabilities.maxImageExtent.width, actualExtent.width)
		);
		actualExtent.height = (std::max)(
			capabilities.minImageExtent.height,
			(std::min)(capabilities.maxImageExtent.height, actualExtent.height)
		);

		return actualExtent;
	}
}

uint32_t SwapChain::chooseImageCount(VkSurfaceCapabilitiesKHR capabilities)
{
	uint32_t imageCount = capabilities.minImageCount + 1;
	if (capabilities.maxImageCount > 0 &&
		imageCount > capabilities.maxImageCount)
	{
		imageCount = capabilities.maxImageCount;
	}

	return imageCount;
}

void SwapChain::initImages()
{
	// real count of images can be greater than requsted
	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);  // get count
	images.resize(imageCount);
	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, images.data());  // get images
}

void SwapChain::createImageViews()
{
	imageViews.resize(imageCount);

	for (uint32_t i = 0; i < imageCount; i++)
	{
		SwapChainImage image(device, images[i], imageFormat);

		VkImageSubresourceRange subresourceRange{
			VK_IMAGE_ASPECT_COLOR_BIT,	// aspectMask;
			0,							// baseMipLevel;
			1,							// levelCount;
			0,							// baseArrayLayer;
			1,							// layerCount;
		};

		imageViews[i] = image.getImageView(subresourceRange);
	}
}
