#pragma once

#include <vector>
#include "Instance.h"
#include "Surface.h"
#include "Device.h"
#include "SwapChain.h"
#include "RenderPass.h"
#include "Scene.h"
#include "DescriptorPool.h"
#include "Settings.h"
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

class Engine
{
public:
    Engine(HWND hWnd, VkExtent2D frameExtent, Settings settings);

	~Engine();

	void setMinimized(bool minimized);

	Camera* getCamera() const;

	void drawFrame();

	void resize(VkExtent2D newExtent);

private:
	Instance *instance;

	Surface *surface;

	Device *device;

	SwapChain *swapChain;

	RenderPassesMap renderPasses;

	Scene *scene;

	DescriptorPool *descriptorPool;

	std::vector<VkCommandBuffer> graphicsCommands;

	VkSemaphore imageAvailable{};
	VkSemaphore renderingFinished{};

	bool ssaoEnabled;

	bool minimized = false;

	void createRenderPasses(uint32_t shadowsDim);

	void initGraphicsCommands();

	void beginRenderPass(VkCommandBuffer commandBuffer, RenderPassType type, uint32_t framebufferIndex);

	void recordRenderPassCommands(VkCommandBuffer commandBuffer, RenderPassType type, uint32_t framebufferIndex);

	static void createSemaphore(VkDevice device, VkSemaphore &semaphore);
};

