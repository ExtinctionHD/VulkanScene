#pragma once

#include <vulkan/vulkan.h>
#include "QueueFamilyIndices.h"
#include "SurfaceSupportDetails.h"

// contains logical (can be cast to it) and physical device
class Device
{
public:
	VkDevice device;  // logical device (representation of GPU for vulkan)

	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;  // GPU

	SurfaceSupportDetails surfaceSupportDetails;  // detail of picked GPU

	QueueFamilyIndices queueFamilyIndices;  // suitable indices on picked GPU

	// pick physical and create logical device
	Device(
		VkInstance instance,
		VkSurfaceKHR surface,
		std::vector<const char *> requiredLayers
	);

	// destroy device
	~Device();

private:
	const std::vector<const char*> extensions =
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	std::vector<const char*> layers;

	VkQueue graphicsQueue;	// for drawing graphics
	VkQueue presentQueue;	// for presenting it on surface

	void pickPhysicalDevice(
		VkInstance instance, 
		VkSurfaceKHR surface
	);

	// has all required queue families,
	// support this surface (capabilities, formats, present modes)
	// all required extensions and layers are available
	static bool isPhysicalDeviceSuitable(
		VkPhysicalDevice device, 
		VkSurfaceKHR surface, 
		std::vector<const char *> requiredLayers, 
		std::vector<const char *> requiredExtensions
	);

	// layers must be supported not only by instance, but also by GPU
	static bool checkDeviceLayerSupport(VkPhysicalDevice device, std::vector<const char *> requiredLayers);

	// some extension to instance, some to GPU
	static bool checkDeviceExtensionSupport(VkPhysicalDevice device, std::vector<const char *> requiredExtensions);

	void createLogicalDevice(VkSurfaceKHR surface);
};

