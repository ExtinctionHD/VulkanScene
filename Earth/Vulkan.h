#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

#include <vector>

#include "VkDeleter.h"

class Vulkan
{
public:
	void init();

private:
	const std::vector<const char *> validationLayers =
	{
		"VK_LAYER_LUNARG_standard_validation"
	};

#ifdef _DEBUG
	const bool enableValidationLayers = true;
#else
	const bool enableValidationLayers = false;
#endif

	const std::vector<const char *> extensions =
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	VkDeleter<VkInstance> instance{ vkDestroyInstance };

	void createInstance();
};

