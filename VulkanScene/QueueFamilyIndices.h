#pragma once

#include <vulkan/vulkan.h>

class QueueFamilyIndices
{
public:
	// queue family indices
	int graphics = -1;	// for drawing
	int present = -1;	// for presenting on surface

	// base constructor
	QueueFamilyIndices() = default;

    // try to find required queue families and save they indices
	QueueFamilyIndices(VkPhysicalDevice device, VkSurfaceKHR surface);

	// this device have all required queue families (for this surface)
	bool isComplete() const;

};

