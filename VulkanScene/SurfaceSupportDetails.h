#pragma once

#include <vulkan/vulkan.h>
#include <vector>

class SurfaceSupportDetails
{
public:
	SurfaceSupportDetails() = default;

	SurfaceSupportDetails(VkPhysicalDevice device, VkSurfaceKHR surface);

	VkSurfaceCapabilitiesKHR getCapabilities() const;

	std::vector<VkSurfaceFormatKHR> getFormats() const;

	std::vector<VkPresentModeKHR> getPresentModes() const;

	bool isSuitable() const;

private:
	VkSurfaceCapabilitiesKHR capabilities;

	std::vector<VkSurfaceFormatKHR> formats;

	std::vector<VkPresentModeKHR> presentModes;

};

