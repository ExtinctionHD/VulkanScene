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
	// слои проверок Vulkan
	const std::vector<const char *> validationLayers =
	{
		"VK_LAYER_LUNARG_standard_validation"  // набор стандартных слоев
	};

#ifdef _DEBUG  // слои проверок включаютс€ толбко в отладочном режиме
	const bool enableValidationLayers = true;
#else
	const bool enableValidationLayers = false;
#endif

	VkDeleter<VkInstance> instance{ vkDestroyInstance };  // экземл€р Vulkan

	void createInstance();  // создание экземпл€ра

	// проверки поддержки экземпл€ром слоев и расширений
	static bool checkInstanceLayerSupport(std::vector<const char *> requiredLayers);
	static bool checkInstanceExtensionSupport(std::vector<const char *> requiredExtensions);

	std::vector<const char *> getRequiredExtensions();  // требуемые приложением расширени€
};

