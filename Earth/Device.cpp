#include <vector>
#include "Logger.h"
#include <set>

#include "Device.h"

// public:

Device::Device(VkInstance instance, VkSurfaceKHR surface, std::vector<const char *> requiredLayers)
{
	layers = requiredLayers;

	pickPhysicalDevice(instance, surface);

	// save properties of picked device
	surfaceSupportDetails = SurfaceSupportDetails(physicalDevice, surface);
	queueFamilyIndices = QueueFamilyIndices(physicalDevice, surface);

	createLogicalDevice(surface);
}

Device::~Device()
{
	vkDestroyDevice(device, nullptr);
}

// private:

void Device::pickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface)
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);  // get count

	if (deviceCount == 0)
	{
		LOGGER_FATAL(Logger::NO_GPU_WITH_VULKAN_SUPPORT);
	}

	std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices.data());  // get devices

	// check each available gpu with vulkan support
	for (const auto& device : physicalDevices)
	{
		if (isPhysicalDeviceSuitable(device, surface, layers, extensions))
		{
			physicalDevice = device;
		}
	}

	if (physicalDevice == VK_NULL_HANDLE)
	{
		LOGGER_FATAL(Logger::NO_SUITABLE_GPU);
	}
}

bool Device::isPhysicalDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface, std::vector<const char *> requiredLayers, std::vector<const char *> requiredExtensions)
{
	QueueFamilyIndices indices(device, surface);
	SurfaceSupportDetails details(device, surface);

	bool layerSupport = checkDeviceLayerSupport(device, requiredLayers);

	bool extensionSupport = checkDeviceExtensionSupport(device, requiredExtensions);

	return indices.isComplete() && details.isSuitable() && layerSupport && extensionSupport;
}

bool Device::checkDeviceLayerSupport(VkPhysicalDevice device, std::vector<const char*> requiredLayers)
{
	uint32_t layerCount;
	vkEnumerateDeviceLayerProperties(device, &layerCount, nullptr);  // get count

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateDeviceLayerProperties(device, &layerCount, availableLayers.data());  // get layers

	std::set<std::string> requiredLayerSet(requiredLayers.begin(), requiredLayers.end());

	for (const auto& layer : availableLayers)
	{
		requiredLayerSet.erase(layer.layerName);
	}

	// empty if all required layers are supported by device
	return requiredLayerSet.empty();
}

bool Device::checkDeviceExtensionSupport(VkPhysicalDevice device, std::vector<const char*> requiredExtensions)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);  // get count

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());  // get extensions

	std::set<std::string> requiredExtensionSet(requiredExtensions.begin(), requiredExtensions.end());

	for (const auto& layer : availableExtensions)
	{
		requiredExtensionSet.erase(layer.extensionName);
	}

	// empty if all required extensions are supported by device
	return requiredExtensionSet.empty();
}

void Device::createLogicalDevice(VkSurfaceKHR surface)
{
	QueueFamilyIndices indices(physicalDevice, surface);

	// graphics and present families can the same
	std::set<int> uniqueQueueFamilyIndices =
	{
		indices.graphics,
		indices.present
	};

	// info about each unique queue famliy
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	float queuePriority = 1.0f;
	for (int queueFamilyIndex : uniqueQueueFamilyIndices)
	{
		VkDeviceQueueCreateInfo queueCreateInfo =
		{
			VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,	// sType
			nullptr,									// pNext
			0,											// flags
			queueFamilyIndex,							// queueFamilyIndex
			1,											// queueCount
			&queuePriority								// pQueuePriorities
		};

		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkDeviceCreateInfo deviceCreateInfo =
	{
		VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,	// sType;
		nullptr,								// pNext;
		0,						                // flags;
		queueCreateInfos.size(),				// queueCreateInfoCount;
		queueCreateInfos.data(),				// pQueueCreateInfos;
		layers.size(),							// enabledLayerCount;
		layers.data(),							// ppEnabledLayerNames;
		extensions.size(),						// enabledExtensionCount;
		extensions.data(),						// ppEnabledExtensionNames;
		nullptr									// pEnabledFeatures;
	};

	VkResult result = vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device);
	if (result != VK_SUCCESS)
	{
		LOGGER_FATAL(Logger::FAILED_TO_CREATE_LOGICAL_DEVICE);
	}

	// save queue handlers
	vkGetDeviceQueue(device, indices.graphics, 0, &graphicsQueue);
	vkGetDeviceQueue(device, indices.present, 0, &presentQueue);
}

