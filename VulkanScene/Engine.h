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
	typedef std::map<RenderPassType, std::vector<VkCommandBuffer>> GraphicsCommands;

	Instance *instance;

	Surface *surface;

	Device *device;

	SwapChain *swapChain;

	RenderPassesMap renderPasses;

	Scene *scene;

	DescriptorPool *descriptorPool;

	GraphicsCommands graphicsCommands;

	VkSemaphore imageAvailableSemaphore;
	std::vector<VkSemaphore> stageFinishedSemaphores;

	bool minimized = false;

	void createRenderPasses(uint32_t shadowsDim);

	void initGraphicsCommands();

	void recordRenderPassCommands(RenderPassType type, uint32_t commandBufferIndex, uint32_t renderCount);

	void beginRenderPass(RenderPassType type, uint32_t commandBufferIndex, uint32_t framebufferIndex);

	static void createSemaphore(VkDevice device, VkSemaphore &semaphore);
};

