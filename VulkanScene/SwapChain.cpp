#include <algorithm>
#include "SurfaceSupportDetails.h"
#include "Device.h"
#include "SwapChainImage.h"
#include <cassert>

#include "SwapChain.h"

// public:

SwapChain::SwapChain(Device *device, VkSurfaceKHR surface, VkExtent2D surfaceExtent)
{
	this->device = device;
	this->surface = surface;

	create(surfaceExtent);
	createImageViews();
}

SwapChain::~SwapChain()
{
	cleanup();
}

VkSwapchainKHR SwapChain::get() const
{
	return swapChain;
}

std::vector<VkImageView> SwapChain::getImageViews() const
{
	return imageViews;
}

VkExtent2D SwapChain::getExtent() const
{
	return extent;
}

uint32_t SwapChain::getImageCount() const
{
	return images.size();
}

VkFormat SwapChain::getImageFormat() const
{
    return imageFormat;
}

void SwapChain::recreate(VkExtent2D newExtent)
{
	cleanup();

	create(newExtent);

	createImageViews();
}

// private:

void SwapChain::create(VkExtent2D surfaceExtent)
{
    // get necessary swapchain properties
	const SurfaceSupportDetails details = device->getSurfaceSupportDetails();
    const VkSurfaceFormatKHR surfaceFormat = chooseSurfaceFormat(details.getFormats());
    const VkPresentModeKHR presentMode = choosePresentMode(details.getPresentModes());
	const VkSurfaceCapabilitiesKHR surfaceCapabilities = details.getCapabilities();
	const uint32_t minImageCount = chooseMinImageCount(surfaceCapabilities);

	extent = chooseExtent(details.getCapabilities(), surfaceExtent);
	imageFormat = surfaceFormat.format;

	VkSwapchainCreateInfoKHR createInfo{
		VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		nullptr,
		0,
		surface,
		minImageCount,
		surfaceFormat.format,
		surfaceFormat.colorSpace,
		extent,
		1,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		VK_SHARING_MODE_EXCLUSIVE,
		0,
		nullptr,
		surfaceCapabilities.currentTransform,			
		VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,				
		presentMode,									
		true,										
		nullptr,										
	};

	// concurrent sharing mode only when using different queue families
    const QueueFamilyIndices queueFamilyIndices = device->getQueueFamilyIndices();
	std::vector<uint32_t> indices{
		queueFamilyIndices.getGraphics(),
		queueFamilyIndices.getPresent()
	};
	if (indices[0] != indices[1])
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = indices.size();
		createInfo.pQueueFamilyIndices = indices.data();
	}

    const VkResult result = vkCreateSwapchainKHR(device->get(), &createInfo, nullptr, &swapChain);
	assert(result == VK_SUCCESS);

	saveImages(minImageCount);
}

VkSurfaceFormatKHR SwapChain::chooseSurfaceFormat(std::vector<VkSurfaceFormatKHR> availableFormats) const
{
	// can choose any format
	if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED)
	{
		return PREFERRED_PRESENT_FORMAT;
	}

	// try to found preferred format from available
	for (const auto &availableFormat : availableFormats)
	{
		if (availableFormat.format == PREFERRED_PRESENT_FORMAT.format &&
			availableFormat.colorSpace == PREFERRED_PRESENT_FORMAT.colorSpace)
		{
			return availableFormat;
		}
	}

	// first available format
	return availableFormats[0];
}

VkPresentModeKHR SwapChain::choosePresentMode(std::vector<VkPresentModeKHR> availablePresentModes) const
{
	for (const auto &availablePresentMode : availablePresentModes)
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

    // extent height and width: (minAvailable <= extent <= maxAvailable) && (extent <= actualExtent)
    actualExtent.width = (std::max)(
        capabilities.minImageExtent.width,
        (std::min)(capabilities.maxImageExtent.width, actualExtent.width));
    actualExtent.height = (std::max)(
        capabilities.minImageExtent.height,
        (std::min)(capabilities.maxImageExtent.height, actualExtent.height));

    return actualExtent;
}

uint32_t SwapChain::chooseMinImageCount(VkSurfaceCapabilitiesKHR capabilities)
{
	uint32_t imageCount = capabilities.minImageCount + 1;
	if (capabilities.maxImageCount > 0 &&
		imageCount > capabilities.maxImageCount)
	{
		imageCount = capabilities.maxImageCount;
	}

	return imageCount;
}

void SwapChain::saveImages(uint32_t imageCount)
{
	// real count of images can be greater than requested
	vkGetSwapchainImagesKHR(device->get(), swapChain, &imageCount, nullptr);  // get count
	images.resize(imageCount);
	vkGetSwapchainImagesKHR(device->get(), swapChain, &imageCount, images.data());  // get images
}

void SwapChain::createImageViews()
{
	imageViews.resize(getImageCount());

	for (uint32_t i = 0; i < getImageCount(); i++)
	{

		const VkImageSubresourceRange subresourceRange{
			VK_IMAGE_ASPECT_COLOR_BIT,
			0,
			1,
			0,
			1,
		};

		SwapChainImage image(device, images[i], imageFormat, subresourceRange);

		imageViews[i] = image.getView();
	}
}

void SwapChain::cleanup()
{
	for (auto imageView : imageViews)
    {
		vkDestroyImageView(device->get(), imageView, nullptr);
	}

	vkDestroySwapchainKHR(device->get(), swapChain, nullptr);
}
