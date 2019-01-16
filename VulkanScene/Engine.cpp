#include "ShaderModule.h"
#include "AssimpModel.h"
#include "FinalRenderPass.h"
#include "DepthRenderPass.h"
#include "GeometryRenderPass.h"
#include "LightingRenderPass.h"
#include "SsaoRenderPass.h"

#include "Engine.h"

// public:

Engine::Engine(HWND hWnd, VkExtent2D frameExtent, Settings settings)
{
	const std::string validationLayer = "VK_LAYER_LUNARG_standard_validation";
    std::vector<const char*> requiredLayers;
#ifdef _DEBUG 
	requiredLayers.push_back(validationLayer.c_str());
#endif

	const std::vector<const char*> extensions{
		VK_KHR_SURFACE_EXTENSION_NAME,
        "VK_KHR_win32_surface"
	};

	ssaoEnabled = settings.ssaoEnabled;

	instance = new Instance(requiredLayers, extensions);
	surface = new Surface(instance->get(), hWnd);
	device = new Device(instance->get(), surface->get(), requiredLayers, settings.sampleCount);
	swapChain = new SwapChain(device, surface->get(), frameExtent);

	createRenderPasses(settings.shadowsDim);

	scene = new Scene(device, swapChain->getExtent(), settings.scenePath, settings.shadowsDistance);
	descriptorPool = new DescriptorPool(device, scene->getBufferCount(), scene->getTextureCount(), scene->getDescriptorSetCount());

	scene->prepareSceneRendering(descriptorPool, renderPasses);

	initGraphicsCommands();

	createSemaphore(device->get(), imageAvailable);
	createSemaphore(device->get(), renderingFinished);
}

Engine::~Engine()
{
	vkDeviceWaitIdle(device->get());
	vkDestroySemaphore(device->get(), imageAvailable, nullptr);
	vkDestroySemaphore(device->get(), renderingFinished, nullptr);

    delete scene;
    delete descriptorPool;
    for (auto [type, renderPass] : renderPasses)
    {
        delete renderPass;
    }
    delete swapChain;
    delete device;
    delete surface;
    delete instance;
}

void Engine::setMinimized(bool minimized)
{
	this->minimized = minimized;
}

Camera* Engine::getCamera() const
{
	return scene->getCamera();
}

void Engine::drawFrame()
{
	scene->updateScene();

	if (minimized)
	{
		return;
	}

	uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(
        device->get(),
        swapChain->get(),
        UINT64_MAX,
        imageAvailable,
        nullptr,
        &imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		resize(swapChain->getExtent());
		return;
	}
    assert(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR);

    std::vector<VkSemaphore> waitSemaphores{ imageAvailable };
	std::vector<VkPipelineStageFlags> waitStages{ VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	std::vector<VkSemaphore> signalSemaphores{ renderingFinished };
	VkSubmitInfo submitInfo{
		VK_STRUCTURE_TYPE_SUBMIT_INFO,
		nullptr,	
		uint32_t(waitSemaphores.size()),
		waitSemaphores.data(),
		waitStages.data(),
		1,
		&graphicsCommands[imageIndex],
		uint32_t(signalSemaphores.size()),
		signalSemaphores.data(),	
	};

	result = vkQueueSubmit(device->getGraphicsQueue(), 1, &submitInfo, nullptr);
	assert(result == VK_SUCCESS);

	std::vector<VkSwapchainKHR> swapChains{ swapChain->get() };
	VkPresentInfoKHR presentInfo{
		VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		nullptr,
		uint32_t(signalSemaphores.size()),
		signalSemaphores.data(),
		uint32_t(swapChains.size()),
		swapChains.data(),
		&imageIndex,
		nullptr,
	};

	result = vkQueuePresentKHR(device->getPresentQueue(), &presentInfo);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
	{
		resize(swapChain->getExtent());
	}
	else
	{
		assert(result == VK_SUCCESS);
	}
}

void Engine::resize(VkExtent2D newExtent)
{
	if (!minimized)
	{
		vkDeviceWaitIdle(device->get());

		swapChain->recreate(newExtent);

		for (auto [type, renderPass] : renderPasses)
		{
			if (type != DEPTH)
			{
				renderPass->recreate(swapChain->getExtent());
			}
		}

		scene->updateDescriptorSets(descriptorPool, renderPasses);
		scene->resizeExtent(swapChain->getExtent());

		initGraphicsCommands();
	}
}

// private:

void Engine::createRenderPasses(uint32_t shadowsDim)
{
	const VkExtent2D depthTextureExtent = { shadowsDim, shadowsDim };

	renderPasses.insert({ DEPTH, new DepthRenderPass(device, depthTextureExtent) });
	renderPasses.insert({ GEOMETRY, new GeometryRenderPass(device, swapChain->getExtent()) });
	renderPasses.insert({ SSAO, new SsaoRenderPass(device, swapChain->getExtent()) });
	renderPasses.insert({ SSAO_BLUR, new SsaoRenderPass(device, swapChain->getExtent()) });
	renderPasses.insert({ LIGHTING, new LightingRenderPass(device, swapChain) });
	renderPasses.insert({ FINAL, new FinalRenderPass(device, swapChain) });

    for (auto [type, renderPass] : renderPasses)
    {
        if (type == FINAL)
        {
			dynamic_cast<FinalRenderPass*>(renderPass)->saveRenderPasses(
				dynamic_cast<GeometryRenderPass*>(renderPasses.at(GEOMETRY)),
				dynamic_cast<LightingRenderPass*>(renderPasses.at(LIGHTING)));
        }
		renderPass->create();
    }
}

void Engine::initGraphicsCommands()
{
    const VkCommandPool commandPool = device->getCommandPool();

	if (!graphicsCommands.empty())
	{
		vkFreeCommandBuffers(device->get(), commandPool, uint32_t(graphicsCommands.size()), graphicsCommands.data());
	}

	graphicsCommands.resize(swapChain->getImageCount());

	VkCommandBufferAllocateInfo allocInfo{
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		nullptr,
		commandPool,
		VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		uint32_t(graphicsCommands.size()),
	};

	VkResult result = vkAllocateCommandBuffers(device->get(), &allocInfo, graphicsCommands.data());
	assert(result == VK_SUCCESS);

	for (size_t i = 0; i < graphicsCommands.size(); i++)
	{
		VkCommandBufferBeginInfo beginInfo{
			VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			nullptr,
			VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
			nullptr,
		};

		result = vkBeginCommandBuffer(graphicsCommands[i], &beginInfo);
		assert(result == VK_SUCCESS);

		recordRenderPassCommands(graphicsCommands[i], DEPTH, 0);
		recordRenderPassCommands(graphicsCommands[i], GEOMETRY, 0);
        if (ssaoEnabled)
        {
			recordRenderPassCommands(graphicsCommands[i], SSAO, 0);
			recordRenderPassCommands(graphicsCommands[i], SSAO_BLUR, 0);
        }
		recordRenderPassCommands(graphicsCommands[i], LIGHTING, 0);
		recordRenderPassCommands(graphicsCommands[i], FINAL, uint32_t(i));

		result = vkEndCommandBuffer(graphicsCommands[i]);

		assert(result == VK_SUCCESS);
	}
}

void Engine::beginRenderPass(VkCommandBuffer commandBuffer, RenderPassType type, uint32_t framebufferIndex)
{
	const VkRect2D renderArea{
		{ 0, 0 },
		renderPasses.at(type)->getExtent()
	};

    auto clearValues = renderPasses.at(type)->getClearValues();

	VkRenderPassBeginInfo renderPassBeginInfo{
	    VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
	    nullptr,
	    renderPasses.at(type)->get(), 
	    renderPasses.at(type)->getFramebuffers()[framebufferIndex],
	    renderArea,
	    uint32_t(clearValues.size()),
		clearValues.data()
	};

	vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void Engine::recordRenderPassCommands(VkCommandBuffer commandBuffer, RenderPassType type, uint32_t framebufferIndex)
{
	beginRenderPass(commandBuffer, type, framebufferIndex);

	scene->render(commandBuffer, type);

	vkCmdEndRenderPass(commandBuffer);
}

void Engine::createSemaphore(VkDevice device, VkSemaphore &semaphore)
{
	if (semaphore)
	{
		vkDestroySemaphore(device, semaphore, nullptr);
	}

	VkSemaphoreCreateInfo createInfo{
		VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		nullptr,
		0,
	};

    const VkResult result = vkCreateSemaphore(device, &createInfo, nullptr, &semaphore);
	assert(result == VK_SUCCESS);
}





