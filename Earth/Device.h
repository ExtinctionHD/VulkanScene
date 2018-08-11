#pragma once

#include <vulkan/vulkan.h>

#include "VkDeleter.h"

// contains logical (can be cast to it) and physical device
class Device
{
public:
	// pick physical and create logical device
	void init(
		VkInstance instance, 
		VkSurfaceKHR surface, 
		std::vector<const char *> requiredLayers
	);

	operator VkDevice();  // cast to logical device

	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;  // GPU

private:
	const std::vector<const char*> extensions =
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	std::vector<const char*> layers;

	// logical device (representation of GPU for vulkan)
	VkDeleter<VkDevice> device{ vkDestroyDevice };

	VkQueue graphicsQueue;	// for drawing graphics
	VkQueue presentQueue;	// for presenting it on surface

	void pickPhysicalDevice(
		VkInstance instance, 
		VkSurfaceKHR surface
	);

	static bool isPhysicalDeviceSuitable(
		VkPhysicalDevice device, 
		VkSurfaceKHR surface, 
		std::vector<const char *> requiredLayers, 
		std::vector<const char *> requiredExtensions
	);

	static bool checkDeviceLayerSupport(VkPhysicalDevice device, std::vector<const char *> requiredLayers);

	static bool checkDeviceExtensionSupport(VkPhysicalDevice device, std::vector<const char *> requiredExtensions);

	void createLogicalDevice(VkSurfaceKHR surface);
};

