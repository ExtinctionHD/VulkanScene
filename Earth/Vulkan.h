#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include "Instance.h"
#include "Device.h"
#include "SwapChain.h"
#include "RenderPass.h"
#include "GraphicsPipeline.h"
#include "Scene.h"
#include "DescriptorPool.h"
#include "Controller.h"

// graphic API class that create all necessary objects
// and set this as window user pointer
class Vulkan
{
public:
	// create all required objects
	Vulkan(GLFWwindow *window, VkExtent2D frameExtent);

	// destroy objects: surface, callback, instance
	~Vulkan();

	// executes graphics commands and present result image on window surface
	void drawFrame();

	// rebuild swapchain and all dependent objects for new extension
	void resize(VkExtent2D newExtent);

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

	// color that clear each frame
	const VkClearColorValue clearColor = { 0, 0, 0, 1 };

	Instance *pInstance;

	// surface object for presentation
	VkSurfaceKHR surface;

	// logical and physical device
	Device *pDevice;

	// swapchain object and its images
	SwapChain *pSwapChain;

	RenderPass *pRenderPass;

	// drawing scene
	Scene *pScene;

	DescriptorPool *pDescriptorPool;

	std::vector<VkCommandBuffer> graphicCommands;

	// synchronizing objects
	VkSemaphore imageAvailable = VK_NULL_HANDLE;
	VkSemaphore renderingFinished = VK_NULL_HANDLE;

	void createSurface(GLFWwindow *window);

	// initialize rendering commands
	void initGraphicCommands();

	static void createSemaphore(VkDevice device, VkSemaphore& semaphore);
};

