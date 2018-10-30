#pragma once

#include <vulkan/vulkan.h>
#include "QueueFamilyIndices.h"
#include "SurfaceSupportDetails.h"

// contains logical (can be cast to it) and physical device
class Device
{
public:
	VkDevice device{};  // logical device (representation of GPU for vulkan)

	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;  // GPU
		
	VkCommandPool commandPool{};  // pool of command buffers of this device

	// pick physical and create logical device
	Device(
		VkInstance instance,
		VkSurfaceKHR surface,
		std::vector<const char *> requiredLayers
	);

	// destroy device
	~Device();

	VkQueue graphicsQueue{};	// for drawing graphics
	VkQueue presentQueue{};	// for presenting it on pSurface

	// returns index of memory type with such properties (for this physical device)
	uint32_t findMemoryTypeIndex(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;

	// returns first supported format (for this physical device)
	VkFormat findSupportedFormat(
		std::vector<VkFormat> requestedFormats,
		VkImageTiling tiling,
		VkFormatFeatureFlags features
	) const;

	SurfaceSupportDetails getSurfaceSupportDetails() const;  // detail of picked GPU

	QueueFamilyIndices getQueueFamilyIndices() const;  // suitable indices on picked GPU

	VkSampleCountFlagBits getSampleCount() const;

	// returns command buffer to write one time commands
	VkCommandBuffer beginOneTimeCommands() const;

	// ends command buffer and submit it to graphics queue
	void endOneTimeCommands(VkCommandBuffer commandBuffer) const;

private:
	const std::vector<const char*> EXTENSIONS =
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	std::vector<const char*> layers;

	VkSurfaceKHR surface;  // properties of the device are calculated for this pSurface

	VkSampleCountFlagBits sampleCount;

	void pickPhysicalDevice(
		VkInstance instance, 
		VkSurfaceKHR surface
	);

	// has all required queue families,
	// support this pSurface (capabilities, formats, present modes)
	// all required extensions and layers are available
	static bool isPhysicalDeviceSuitable(
		VkPhysicalDevice device, 
		VkSurfaceKHR surface, 
		std::vector<const char *> requiredLayers, 
		std::vector<const char *> requiredExtensions
	);

	VkSampleCountFlagBits getMaxSupportedSampleCount() const;  // max sample count supported by picked GPU

	// layers must be supported not only by instance, but also by GPU
	static bool checkDeviceLayerSupport(VkPhysicalDevice device, std::vector<const char *> requiredLayers);

	// some extension to instance, some to GPU
	static bool checkDeviceExtensionSupport(VkPhysicalDevice device, std::vector<const char *> requiredExtensions);

	void createLogicalDevice(VkSurfaceKHR surface);

	void createCommandPool(VkPhysicalDevice physicalDevice);
};

