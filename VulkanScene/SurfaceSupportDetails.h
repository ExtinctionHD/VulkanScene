#pragma once

#include <vulkan/vulkan.h>
#include <vector>

class SurfaceSupportDetails
{
public:
	VkSurfaceCapabilitiesKHR capabilities{} {};

	std::vector<VkSurfaceFormatKHR> formats;

	std::vector<VkPresentModeKHR> presentModes;

	// base constructor
	SurfaceSupportDetails() {}

	// initialize all details
	SurfaceSupportDetails(VkPhysicalDevice device, VkSurfaceKHR surface);

	// this device is suitable for this pSurface
	bool isSuitable() const;

};

