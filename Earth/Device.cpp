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

	createCommandPool(physicalDevice);
}

Device::~Device()
{
	vkDestroyCommandPool(device, commandPool, nullptr);
	vkDestroyDevice(device, nullptr);
}

uint32_t Device::findMemoryTypeIndex(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return i;
		}
	}

	LOGGER_FATAL(Logger::FAILED_TO_FIND_MEMORY_TYPE);
}

VkFormat Device::findSupportedFormat(std::vector<VkFormat> requestedFormats, VkImageTiling tiling, VkFormatFeatureFlags features)
{
	for (VkFormat format : requestedFormats)
	{
		VkFormatProperties properties;
		vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &properties);

		// checks support of this format with linear tiling
		if (tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & features) == features)
		{
			return format;
		}

		// checks support of this format with optimal tiling
		if (tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & features) == features)
		{
			return format;
		}

		LOGGER_FATAL(Logger::FAILED_TO_FIND_SUPPORTED_FORMAT);
	}
}

VkCommandBuffer Device::beginOneTimeCommands()
{
	VkCommandBuffer commandBuffer;

	VkCommandBufferAllocateInfo allocInfo{
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,	// sType;
		nullptr,										// pNext;
		commandPool,									// commandPool;
		VK_COMMAND_BUFFER_LEVEL_PRIMARY,				// level;
		1,												// commandBufferCount;
	};

	VkResult result = vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);
	if (result != VK_SUCCESS)
	{
		LOGGER_FATAL(Logger::FAILED_TO_ALLOC_COMMAND_BUFFERS);
	}

	VkCommandBufferBeginInfo beginInfo{
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,	// sType;
		nullptr,										// pNext;
		VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,	// flags;
		nullptr,										// pInheritanceInfo;
	};

	result = vkBeginCommandBuffer(commandBuffer, &beginInfo);
	if (result != VK_SUCCESS)
	{
		LOGGER_FATAL(Logger::FAILED_TO_BEGIN_COMMAND_BUFFER);
	}

	return commandBuffer;
}

void Device::endOneTimeCommands(VkCommandBuffer commandBuffer)
{
	VkResult result = vkEndCommandBuffer(commandBuffer);
	if (result != VK_SUCCESS)
	{
		LOGGER_FATAL(Logger::FAILED_TO_END_COMMAND_BUFFER);
	}

	VkSubmitInfo submitInfo{
		VK_STRUCTURE_TYPE_SUBMIT_INFO,	// sType;
		nullptr,						// pNext;
		0,								// waitSemaphoreCount;
		nullptr,						// pWaitSemaphores;
		nullptr,						// pWaitDstStageMask;
		1,								// commandBufferCount;
		&commandBuffer,					// pCommandBuffers;
		0,								// signalSemaphoreCount;
		nullptr,						// pSignalSemaphores;
	};

	vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(graphicsQueue);  // TODO: replace wait idle to signal semophore

	vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
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
		if (isPhysicalDeviceSuitable(device, surface, layers, EXTENSIONS))
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
		EXTENSIONS.size(),						// enabledExtensionCount;
		EXTENSIONS.data(),						// ppEnabledExtensionNames;
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

void Device::createCommandPool(VkPhysicalDevice physicalDevice)
{
	VkCommandPoolCreateInfo createInfo{
		VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,	// sType;
		nullptr,									// pNext;
		0,											// flags;
		queueFamilyIndices.graphics					// queueFamilyIndex;
	};

	VkResult result = vkCreateCommandPool(device, &createInfo, nullptr, &commandPool);
	if (result != VK_SUCCESS)
	{
		LOGGER_FATAL(Logger::FAILED_TO_CREATE_COMMAND_POOL);
	}
}