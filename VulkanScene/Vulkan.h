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
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

// graphic API class that create all necessary objects
// and set this as window user pointer
class Vulkan
{
public:
	// create all required objects
	Vulkan(
		HINSTANCE hInstance,
		HWND hWnd,
		VkExtent2D frameExtent,
		VkSampleCountFlagBits sampleCount,
		bool ssaoEnabled,
		uint32_t shadowsDim,
		float shadowsRadius,
		const std::string &lightingFile
	);

	// destroy objects: pSurface, callback, instance
	~Vulkan();

	bool minimized = false;

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
	VkSemaphore imageAvailable = VK_NULL_HANDLE;
	VkSemaphore renderingFinished = VK_NULL_HANDLE;

	bool ssaoEnabled;

	void createRenderPasses(uint32_t shadowsDim);

	// initialize rendering commands
	void initGraphicsCommands();

	void beginRenderPass(VkCommandBuffer commandBuffer, RenderPassType type, uint32_t framebufferIndex);

	void recordRenderPassCommands(VkCommandBuffer commandBuffer, RenderPassType type, uint32_t framebufferIndex);

	static void createSemaphore(VkDevice device, VkSemaphore& semaphore);
};

