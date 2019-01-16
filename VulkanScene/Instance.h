#pragma once

#include <vector>
#include <vulkan/vulkan.h>

class Instance
{
public:
	Instance(const std::vector<const char*> &requiredLayers, std::vector<const char*> requiredExtensions);
	~Instance();

	VkInstance get() const;

private:
	VkInstance instance;

	VkDebugReportCallbackEXT callback;

	void createInstance(
        const std::vector<const char*> &requiredLayers,
        const std::vector<const char*> &requiredExtensions);

	static bool checkInstanceLayerSupport(const std::vector<const char*> &requiredLayers);

	static bool checkInstanceExtensionSupport(const std::vector<const char*> &requiredExtensions);

	// functions from extensions (EXT) must be obtained before use
	static VkResult vkCreateDebugReportCallbackEXT(
		VkInstance instance,
		const VkDebugReportCallbackCreateInfoEXT *pCreateInfo,
		const VkAllocationCallbacks *pAllocator,
		VkDebugReportCallbackEXT *pCallback);

	static void vkDestroyDebugReportCallbackEXT(
		VkInstance instance,
		VkDebugReportCallbackEXT callback,
		const VkAllocationCallbacks *pAllocator);

	void createDebugCallback();

	// log messages from vulkan validation layers
	static VKAPI_ATTR VkBool32 VKAPI_CALL validationLayerCallback(
		VkDebugReportFlagsEXT flags,
		VkDebugReportObjectTypeEXT objType,
		uint64_t obj,
		size_t location,
		int32_t code,
		const char *layerPrefix,
		const char *msg,
		void *userData);
};

