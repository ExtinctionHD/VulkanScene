#pragma once

#include <vulkan/vulkan.h>
#include "QueueFamilyIndices.h"
#include "SurfaceSupportDetails.h"

class Device
{
public:
	Device(
		VkInstance instance,
		VkSurfaceKHR surface,
        const std::vector<const char*> &requiredLayers,
		VkSampleCountFlagBits maxRequiredSampleCount);

	~Device();

	VkDevice get() const;

	VkQueue getGraphicsQueue() const;

	VkQueue getPresentQueue() const;

	VkCommandPool getCommandPool() const;

	VkFormatProperties getFormatProperties(VkFormat format) const;

	// returns index of memory type with such properties (for this physical device)
	uint32_t findMemoryTypeIndex(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;

	// returns first supported format (for this physical device)
	VkFormat findSupportedFormat(
		std::vector<VkFormat> requestedFormats,
		VkImageTiling tiling,
		VkFormatFeatureFlags features) const;

	SurfaceSupportDetails getSurfaceSupportDetails() const;

	QueueFamilyIndices getQueueFamilyIndices() const;

	VkSampleCountFlagBits getSampleCount() const;

	// returns command buffer to write one time commands
	VkCommandBuffer beginOneTimeCommands() const;

	// ends command buffer and submit it to graphics queue
	void endOneTimeCommands(VkCommandBuffer commandBuffer) const;

private:
	const std::vector<const char*> EXTENSIONS{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	VkDevice device;

	VkSampleCountFlagBits sampleCount;

	VkPhysicalDevice physicalDevice;  // GPU

	VkSurfaceKHR surface;

	VkQueue graphicsQueue;

	VkQueue presentQueue;

	VkCommandPool commandPool;

    VkPhysicalDevice pickPhysicalDevice(VkInstance instance, const std::vector<const char*> &layers) const;

	// has all required queue families,
	// support this surface (capabilities, formats, present modes)
	// all required extensions and layers are available
	bool physicalDeviceSuitable(
        VkPhysicalDevice device,
        const std::vector<const char*> &requiredLayers,
        const std::vector<const char*> &requiredExtensions) const;

	VkSampleCountFlagBits getMaxSupportedSampleCount(VkPhysicalDevice physicalDevice) const;

	static bool checkDeviceLayerSupport(VkPhysicalDevice device, const std::vector<const char*> &requiredLayers);

	static bool checkDeviceExtensionSupport(VkPhysicalDevice device, const std::vector<const char*> &requiredExtensions);

	void createDevice(const std::vector<const char*> &layers);

	void createCommandPool();
};

