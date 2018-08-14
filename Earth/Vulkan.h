#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include "Device.h"
#include "SwapChain.h"

#include "VkDeleter.h"

class Vulkan
{
public:
	// create all required objects
	void init(GLFWwindow *window);

private:
	const std::vector<const char *> validationLayers =
	{
		"VK_LAYER_LUNARG_standard_validation"  // some debug layers
	};

#ifdef _DEBUG  // validation layers enable only in debug mode
	const bool enableValidationLayers = true;
#else
	const bool enableValidationLayers = false;
#endif

	VkDeleter<VkInstance> instance{ vkDestroyInstance };

	// validation layers callback
	VkDeleter<VkDebugReportCallbackEXT> callback{ instance, vkDestroyDebugReportCallbackEXT };

	// surface object for presentation
	VkDeleter<VkSurfaceKHR> surface{ instance, vkDestroySurfaceKHR };

	// logical and physical device
	Device device;

	// swapchain object and its images
	SwapChain swapChain;

	void createInstance();

	static bool checkInstanceLayerSupport(std::vector<const char *> requiredLayers);
	static bool checkInstanceExtensionSupport(std::vector<const char *> requiredExtensions);

	std::vector<const char *> getRequiredExtensions();

	// functions from extensions (EXT) need to get before use
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

	void createDebugCallback();

	void createSurface(GLFWwindow *window);
};

