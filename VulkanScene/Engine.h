#pragma once

#include <vector>
#include "Instance.h"
#include "Surface.h"
#include "Device.h"
#include "SwapChain.h"
#include "RenderPass.h"
#include "Scene.h"
#include "DescriptorPool.h"
#include <Windows.h>
#include "Settings.h"
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

// graphic API class that create all necessary objects
// and set this as window user pointer
class Engine
{
public:
	// create all required objects
    Engine(
        HWND hWnd,
        VkExtent2D frameExtent,
        Settings settings
	);

	// destroy objects: pSurface, callback, instance
	~Engine();

	bool minimized = false;

	Camera *getCamera() const;

	// executes graphics commands and present result image on window pSurface
	void drawFrame();

	// rebuild swapchain and all dependent objects for new extension
	void resize(VkExtent2D newExtent);

private:
	Instance *pInstance;

	// pSurface object for presentation
	Surface *pSurface;

	// logical and physical device
	Device *pDevice;

	// swapchain object and its images
	SwapChain *pSwapChain;

	RenderPassesMap renderPasses;

	// drawing scene
	Scene *pScene;

	DescriptorPool *pDescriptorPool;

	std::vector<VkCommandBuffer> graphicCommands;

	// synchronizing objects
	VkSemaphore imageAvailable = nullptr;
	VkSemaphore renderingFinished = nullptr;

	bool ssaoEnabled;

	void createRenderPasses(uint32_t shadowsDim);

	// initialize rendering commands
	void initGraphicsCommands();

	void beginRenderPass(VkCommandBuffer commandBuffer, RenderPassType type, uint32_t framebufferIndex);

	void recordRenderPassCommands(VkCommandBuffer commandBuffer, RenderPassType type, uint32_t framebufferIndex);

	static void createSemaphore(VkDevice device, VkSemaphore& semaphore);
};

