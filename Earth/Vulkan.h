#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>

#include "VkDeleter.h"

class Vulkan
{
public:
	void init(GLFWwindow *window);

private:
	// слои проверки Vulkan
	const std::vector<const char *> validationLayers =
	{
		"VK_LAYER_LUNARG_standard_validation"  // набор стандартных слоев
	};

#ifdef _DEBUG  // слои провеки включаютс€ только в отладочном режиме
	const bool enableValidationLayers = true;
#else
	const bool enableValidationLayers = false;
#endif

	VkDeleter<VkInstance> instance{ vkDestroyInstance };  // экземл€р Vulkan

	// callback слоев проверки
	VkDeleter<VkDebugReportCallbackEXT> callback{ instance, vkDestroyDebugReportCallbackEXT };

	// объект поверхности дл€ отображени€
	VkDeleter<VkSurfaceKHR> surface{ instance, vkDestroySurfaceKHR };

	void createInstance();  // создание экземпл€ра

	// проверки поддержки экземпл€ром слоев и расширений
	static bool checkInstanceLayerSupport(std::vector<const char *> requiredLayers);
	static bool checkInstanceExtensionSupport(std::vector<const char *> requiredExtensions);

	std::vector<const char *> getRequiredExtensions();  // требуемые приложением расширени€

	// функции из расширений (EXT) необходимо загрузить перед использованием
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

	void createDebugCallback();  // создание обратного вызова слоев проверки

	void createSurface(GLFWwindow *window);  // создание поверхности поверх окна
};

