#pragma once

#include <vector>
#include <vulkan/vulkan.h>

// core object of vulkan api
class Instance
{
public:
	Instance(std::vector<const char *> requiredLayers, std::vector<const char *> requiredExtensions);
	~Instance();

	VkInstance getInstance();

private:
	VkInstance instance;

	// validation layers callback
	VkDebugReportCallbackEXT callback;

	void createInstance(std::vector<const char *> requiredLayers, std::vector<const char *> requiredExtensions);

	static bool checkInstanceLayerSupport(std::vector<const char *> requiredLayers);
	static bool checkInstanceExtensionSupport(std::vector<const char *> requiredExtensions);

	// functions from extensions (EXT) need to get before use
	static VkResult vkCreateDebugReportCallbackEXT(
		VkInstance instance,
		const VkDebugReportCallbackCreateInfoEXT *pCreateInfo,
		const VkAllocationCallbacks *pAllocator,
		VkDebugReportCallbackEXT *pCallback
	);
	static void vkDestroyDebugReportCallbackEXT(
		VkInstance instance,
		VkDebugReportCallbackEXT callback,
		const VkAllocationCallbacks *pAllocator
	);

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
		void *userData
	);
};

