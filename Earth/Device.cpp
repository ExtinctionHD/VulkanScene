#include <vector>
#include "Logger.h"

#include "Device.h"

void Device::init()
{
}

void Device::pickPhysicalDevice(VkInstance instance)
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
		if (isPhysicalDeviceSuitable(device))
		{
			physicalDevice = device;
		}
	}

	if (physicalDevice == VK_NULL_HANDLE)
	{
		LOGGER_FATAL(Logger::NO_SUITABLE_GPU);
	}
}

bool Device::isPhysicalDeviceSuitable(VkPhysicalDevice device)
{
	return false;
}
