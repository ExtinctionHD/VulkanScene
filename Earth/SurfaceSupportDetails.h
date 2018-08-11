#pragma once

#include <vulkan/vulkan.h>
#include <vector>

class SurfaceSupportDetails
{
public:
	VkSurfaceCapabilitiesKHR capabilities;

	std::vector<VkSurfaceFormatKHR> formats;

	std::vector<VkPresentModeKHR> presentModes;

	// this device suitable for this surface
	bool isSuitable();

	// initialize all details
	void init(VkPhysicalDevice device, VkSurfaceKHR surface);
};

