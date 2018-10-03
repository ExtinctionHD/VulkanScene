#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include "Instance.h"
#include "Device.h"
#include "SwapChain.h"
#include "RenderPass.h"
#include "GraphicsPipeline.h"
#include "DescriptorSet.h"
#include "Image.h"
#include "TextureImage.h"
#include "Model.h"
#include "MvpMatrices.h"
#include "Timer.h"
#include "Lighting.h"
#include "Camera.h"

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

	// moves camera on key press
	void onKeyAction(int key, int action);

	// moves camera on mouse move
	void onMouseMove(float x, float y);

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

	enum Shaders
	{
		mainVert, mainFrag, skyboxVert, skyboxFrag
	};
	// files with shaders code
	const std::vector<std::string> SHADERS_PATHES = {
		File::getExeDir() + "shaders/mainVert.spv",
		File::getExeDir() + "shaders/mainFrag.spv",
		File::getExeDir() + "shaders/skyboxVert.spv",
		File::getExeDir() + "shaders/skyboxFrag.spv"
	};

	Instance *pInstance;

	// surface object for presentation
	VkSurfaceKHR surface;

	// logical and physical device
	Device *pDevice;

	// swapchain object and its images
	SwapChain *pSwapChain;

	// resources for main graphics pipeline
	DescriptorSet *pMainDS;

	// resources for skybox graphics pipeline
	DescriptorSet *pSkyboxDS;

	RenderPass *pRenderPass;

	// graphics pipelines for rendering main objects
	GraphicsPipeline *pMainPipeline;

	// graphics pipeline for rendering skybox
	GraphicsPipeline *pSkyboxPipeline;

	std::vector<VkCommandBuffer> graphicCommands;

	// synchronizing objects
	VkSemaphore imageAvailable = VK_NULL_HANDLE;
	VkSemaphore renderingFinished = VK_NULL_HANDLE;

	void createSurface(GLFWwindow *window);

	// initialize rendering commands
	void initGraphicCommands();

	static void createSemaphore(VkDevice device, VkSemaphore& semaphore);
};

