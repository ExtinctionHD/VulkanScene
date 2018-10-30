#include <set>
#include <iostream>
#include <fstream>
#include "File.h"
#include <cassert>

#include "Instance.h"

Instance::Instance(std::vector<const char *> requiredLayers, std::vector<const char *> requiredExtensions)
{
	if (!requiredLayers.empty())
	{
		requiredExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	}

	createInstance(requiredLayers, requiredExtensions);

	if (!requiredLayers.empty())
	{
		// will write error messages to file
		std::ofstream logFile(File::getExeDir() + "ValidationLayers.log");
		std::cerr.rdbuf(logFile.rdbuf());

		createDebugCallback();
	}
}

Instance::~Instance()
{
	vkDestroyDebugReportCallbackEXT(instance, callback, nullptr);
	vkDestroyInstance(instance, nullptr);
}

VkInstance Instance::getInstance()
{
	return instance;
}

// private:

void Instance::createInstance(std::vector<const char *> requiredLayers, std::vector<const char *> requiredExtensions)
{
	// required layers
	assert(checkInstanceLayerSupport(requiredLayers));

	// required extensions
	assert(checkInstanceExtensionSupport(requiredExtensions));

	// infoabout application for vulkan
	VkApplicationInfo appInfo =
	{
		VK_STRUCTURE_TYPE_APPLICATION_INFO,	// sType
		nullptr,							// pNext
		"VulkanScene",						// pApplicationName
		VK_MAKE_VERSION(1, 0, 0),			// applicationVersion
		"No Engine",						// pEngineName
		VK_MAKE_VERSION(1, 0, 0),			// engineVersion
		VK_API_VERSION_1_0					// apiVersion
	};

	// info for vulkan instance
	VkInstanceCreateInfo createInfo =
	{
		VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,	// sType
		nullptr,								// pNext
		0,										// flags
		&appInfo,								// pApplicationInfo
		requiredLayers.size(),					// enabledLayerCount
		requiredLayers.data(),					// ppEnabledLayerNames
		requiredExtensions.size(),				// enabledExtensionCount
		requiredExtensions.data()				// ppEnabledExtensionNames
	};

	VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
	assert(result == VK_SUCCESS);
}

bool Instance::checkInstanceLayerSupport(std::vector<const char*> requiredLayers)
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);  // get count

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());  // get layers

	std::set<std::string> requiredLayerSet(requiredLayers.begin(), requiredLayers.end());

	for (const auto& layer : availableLayers)
	{
		requiredLayerSet.erase(layer.layerName);
	}

	// empty if all required layers are supported by instance
	return requiredLayerSet.empty();
}

bool Instance::checkInstanceExtensionSupport(std::vector<const char*> requiredExtensions)
{
	uint32_t extensionCount;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);  // get count

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());  // get extensions

	std::set<std::string> requiredExtensionSet(requiredExtensions.begin(), requiredExtensions.end());

	for (const auto& layer : availableExtensions)
	{
		requiredExtensionSet.erase(layer.extensionName);
	}

	// empty if all required extensions are supported by instance
	return requiredExtensionSet.empty();
}

VkResult Instance::vkCreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT * pCreateInfo, const VkAllocationCallbacks * pAllocator, VkDebugReportCallbackEXT * pCallback)
{
	auto func = PFN_vkCreateDebugReportCallbackEXT(vkGetInstanceProcAddr(
		instance, "vkCreateDebugReportCallbackEXT"
	));

	if (func != nullptr)
	{
		return func(instance, pCreateInfo, pAllocator, pCallback);
	}
	else
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void Instance::vkDestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks * pAllocator)
{
	auto func = PFN_vkDestroyDebugReportCallbackEXT(vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT"));
	if (func != nullptr)
	{
		func(instance, callback, pAllocator);
	}
}

void Instance::createDebugCallback()
{
	VkDebugReportCallbackCreateInfoEXT createInfo =
	{
		VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT,			// sType
		nullptr,															// pNext
		VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT,	// flags
		validationLayerCallback,											// pfnCallback
		nullptr																// pUserData
	};

	VkResult result = vkCreateDebugReportCallbackEXT(instance, &createInfo, nullptr, &callback);
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
	void * userData
)
{
	std::cerr << "VALIDATION LAYER | " << msg << std::endl;

	return VK_FALSE;
}