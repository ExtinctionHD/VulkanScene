#include <vector>

#include "QueueFamilyIndices.h"

// pubilc:

bool QueueFamilyIndices::isComplete() const
{
	return graphics >= 0 && present >= 0;
}

QueueFamilyIndices::QueueFamilyIndices(VkPhysicalDevice device, VkSurfaceKHR surface)
{
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);  // get count

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());  // get queue family properties

	// check each queue family in this device
	for (int i = 0; i < queueFamilyCount; i++)
	{
		if (queueFamilies[i].queueCount > 0 && queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			graphics = i;  // this family can draw graphics
		}

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
		if (queueFamilies[i].queueCount > 0 && presentSupport)
		{
			present = i;  // this family can present it on pSurface
		}

		if (isComplete())
		{
			break;
		}
	}
}
