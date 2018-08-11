#pragma once

#include <vulkan/vulkan.h>

#include "VkDeleter.h"

class Device
{
public:
	void init();

private:
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;  // GPU

	// logical device (representation of GPU for vulkan)
	VkDeleter<VkDevice> device{ vkDestroyDevice };

	VkQueue graphicsQueue;
	VkQueue presentQueue;

	void pickPhysicalDevice(VkInstance instance);

	bool isPhysicalDeviceSuitable(VkPhysicalDevice device);
};

