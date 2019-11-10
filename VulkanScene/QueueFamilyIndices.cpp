#include <vector>

#include "QueueFamilyIndices.h"
#include <stdexcept>

// public:

QueueFamilyIndices::QueueFamilyIndices(VkPhysicalDevice device, VkSurfaceKHR surface)
{
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);  // get count

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());  // get queue family properties

	for (uint32_t i = 0; i < queueFamilyCount; i++)
	{
		if (queueFamilies[i].queueCount > 0 && queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			graphics = i;
		}

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
		if (queueFamilies[i].queueCount > 0 && presentSupport)
		{
			present = i;
		}

		if (completed())
		{
			break;
		}
	}
}

uint32_t QueueFamilyIndices::getGraphics() const
{
    if (graphics >= 0)
    {
		return uint32_t(graphics);
    }

	throw std::runtime_error("No required queue family");
}

uint32_t QueueFamilyIndices::getPresent() const
{
	if (present >= 0)
	{
		return uint32_t(present);
	}

	throw std::runtime_error("No required queue family");
}

bool QueueFamilyIndices::completed() const
{
	return graphics >= 0 && present >= 0;
}
