#pragma once

#include <vulkan/vulkan.h>

class QueueFamilyIndices
{
public:
	QueueFamilyIndices() = default;

    // try to find required queue families and save they indices
	QueueFamilyIndices(VkPhysicalDevice device, VkSurfaceKHR surface);

	uint32_t getGraphics() const;

	uint32_t getPresent() const;

	// this device have all required queue families (for this surface)
	bool completed() const;

private:
	// queue family indices
	int graphics = -1;
	int present = -1;
};

