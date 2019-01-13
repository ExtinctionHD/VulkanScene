#include "SurfaceSupportDetails.h"

// public:

SurfaceSupportDetails::SurfaceSupportDetails(VkPhysicalDevice device, VkSurfaceKHR surface)
{
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);  // get count
	if (formatCount > 0)
	{
		formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, formats.data());  // get available surface formats
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
	if (presentModeCount > 0)
	{
		presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, presentModes.data());
	}
}

VkSurfaceCapabilitiesKHR SurfaceSupportDetails::getCapabilities() const
{
	return capabilities;
}

std::vector<VkSurfaceFormatKHR> SurfaceSupportDetails::getFormats() const
{
	return formats;
}

std::vector<VkPresentModeKHR> SurfaceSupportDetails::getPresentModes() const
{
	return presentModes;
}

bool SurfaceSupportDetails::isSuitable() const
{
	return !formats.empty() && !presentModes.empty();
}
