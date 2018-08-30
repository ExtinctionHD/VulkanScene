#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include "Device.h"
#include "SwapChain.h"
#include "Window.h"
#include "GraphicsPipeline.h"
#include "DescriptorSet.h"
#include "Image.h"
#include "TextureImage.h"
#include "Model.h"
#include "MVPmatrices.h"

class Vulkan
{
public:
	// create all required objects
	Vulkan(Window *pWindow);

	// destroy objects: surface, callback, instance
	~Vulkan();

	void drawFrame();

private:
	const std::vector<const char *> VALIDATION_LAYERS =
	{
		"VK_LAYER_LUNARG_standard_validation"  // some debug layers
	};

#ifdef _DEBUG  // validation layers enable only in debug mode
	const bool ENABLE_VALIDATION_LAYERS = true;
#else
	const bool ENABLE_VALIDATION_LAYERS = false;
#endif

	const VkClearColorValue backgroundColor = { 0, 0, 0, 1 };

	VkInstance instance;

	// validation layers callback
	VkDebugReportCallbackEXT callback;

	// surface object for presentation
	VkSurfaceKHR surface;

	// logical and physical device
	Device *pDevice;

	// swapchain object and its images
	SwapChain *pSwapChain;

	// Resources for graphics pipeline
	DescriptorSet *pDescriptorSet;

	GraphicsPipeline *pGraphicsPipeline;

	// resources
	TextureImage *pEarthTexture;	// texture of earth surface
	Model *pEarthModel;				// model of earth
	Buffer *pMVPBuffer;				// buffer containing MVP(model, view, projection) matrices
	MVPmatrices mvp;				// model, view, projection matrices

	std::vector<VkCommandBuffer> graphicCommands;

	VkSemaphore imageAvailable = VK_NULL_HANDLE;
	VkSemaphore renderingFinished = VK_NULL_HANDLE;

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

	void initDescriptorSet();

	void initGraphicCommands();

	static void createSemaphore(VkDevice device, VkSemaphore& semaphore);
};

