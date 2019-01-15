#include <set>
#include <cassert>

#include "Instance.h"

// public:

Instance::Instance(const std::vector<const char*> &requiredLayers, std::vector<const char*> requiredExtensions)
{
	if (!requiredLayers.empty())
	{
		requiredExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	}

	createInstance(requiredLayers, requiredExtensions);

	if (!requiredLayers.empty())
	{
		createDebugCallback();
	}
}

Instance::~Instance()
{
	vkDestroyDebugReportCallbackEXT(instance, callback, nullptr);
	vkDestroyInstance(instance, nullptr);
}

VkInstance Instance::get() const
{
	return instance;
}

// private:

void Instance::createInstance(
    const std::vector<const char*> &requiredLayers,
    const std::vector<const char*> &requiredExtensions)
{
	assert(checkInstanceLayerSupport(requiredLayers));
	assert(checkInstanceExtensionSupport(requiredExtensions));

	VkApplicationInfo appInfo{
		VK_STRUCTURE_TYPE_APPLICATION_INFO,
		nullptr,
		"VulkanScene",
		VK_MAKE_VERSION(1, 0, 0),
		"No Engine",
		VK_MAKE_VERSION(1, 0, 0),
		VK_API_VERSION_1_0
	};

	VkInstanceCreateInfo createInfo{
		VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		nullptr,
		0,
		&appInfo,
		requiredLayers.size(),
		requiredLayers.data(),
		requiredExtensions.size(),
		requiredExtensions.data()
	};

    const VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
	assert(result == VK_SUCCESS);
}

bool Instance::checkInstanceLayerSupport(const std::vector<const char*> &requiredLayers)
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);  // get count

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());  // get layers

	std::set<std::string> requiredLayerSet(requiredLayers.begin(), requiredLayers.end());

	for (const auto &layer : availableLayers)
	{
		requiredLayerSet.erase(layer.layerName);
	}

	// empty if all required layers are supported by instance
	return requiredLayerSet.empty();
}

bool Instance::checkInstanceExtensionSupport(const std::vector<const char*> &requiredExtensions)
{
	uint32_t extensionCount;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);  // get count

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());  // get extensions

	std::set<std::string> requiredExtensionSet(requiredExtensions.begin(), requiredExtensions.end());

	for (const auto &layer : availableExtensions)
	{
		requiredExtensionSet.erase(layer.extensionName);
	}

	// empty if all required extensions are supported by instance
	return requiredExtensionSet.empty();
}

VkResult Instance::vkCreateDebugReportCallbackEXT(
    VkInstance instance,
    const VkDebugReportCallbackCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkDebugReportCallbackEXT *pCallback)
{
    const auto func = PFN_vkCreateDebugReportCallbackEXT(
        vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT"));

	if (func != nullptr)
	{
		return func(instance, pCreateInfo, pAllocator, pCallback);
	}

    return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void Instance::vkDestroyDebugReportCallbackEXT(
    VkInstance instance,
    VkDebugReportCallbackEXT callback,
    const VkAllocationCallbacks *pAllocator)
{
    const auto func = PFN_vkDestroyDebugReportCallbackEXT(vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT"));
	if (func != nullptr)
	{
		func(instance, callback, pAllocator);
	}
}

void Instance::createDebugCallback()
{
	VkDebugReportCallbackCreateInfoEXT createInfo =
	{
		VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT,		
		nullptr,														
		VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT,
		validationLayerCallback,										
		nullptr															
	};

    const VkResult result = vkCreateDebugReportCallbackEXT(instance, &createInfo, nullptr, &callback);
	assert(result == VK_SUCCESS);
}

VKAPI_ATTR VkBool32 VKAPI_CALL Instance::validationLayerCallback(
	VkDebugReportFlagsEXT flags,
	VkDebugReportObjectTypeEXT objType,
	uint64_t obj,
	size_t location,
	int32_t code,
	const char * layerPrefix,
	const char * msg,
	void * userData)
{
	assert(false);

	return false;
}