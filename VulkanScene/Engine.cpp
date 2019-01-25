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

	instance = new Instance(requiredLayers, extensions);
	surface = new Surface(instance->get(), hWnd);
	device = new Device(instance->get(), surface->get(), requiredLayers, settings.sampleCount);
	swapChain = new SwapChain(device, surface->get(), frameExtent);

	createRenderPasses(settings.shadowsDim);

	scene = new Scene(device, swapChain->getExtent(), settings.scenePath);
	descriptorPool = new DescriptorPool(
        device,
        scene->getBufferCount(),
        scene->getTextureCount(),
        scene->getDescriptorSetCount());

	scene->prepareSceneRendering(descriptorPool, renderPasses);

	imageAvailableSemaphore = nullptr;
	createSemaphore(device->get(), imageAvailableSemaphore);

	stageFinishedSemaphores.resize(renderPasses.size(), nullptr);
    for (auto &semaphore : stageFinishedSemaphores)
    {
		createSemaphore(device->get(), semaphore);
    }

	initGraphicsCommands();
}

Engine::~Engine()
{
	vkDeviceWaitIdle(device->get());
	vkDestroySemaphore(device->get(), imageAvailableSemaphore, nullptr);
    for (auto semaphore : stageFinishedSemaphores)
    {
		vkDestroySemaphore(device->get(), semaphore, nullptr);
    }

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

	if (minimized) return;

	uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(
        device->get(),
        swapChain->get(),
        UINT64_MAX,
        imageAvailableSemaphore,
        nullptr,
        &imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		resize(swapChain->getExtent());
		return;
	}
    assert(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR);

    // Depth:
	std::vector<VkSemaphore> signalSemaphores{ stageFinishedSemaphores[DEPTH] };
	VkSubmitInfo submitInfo{
		VK_STRUCTURE_TYPE_SUBMIT_INFO,
		nullptr,
		0,
		nullptr,
		nullptr,
		1,
		&graphicsCommands.at(DEPTH)[0],
		uint32_t(signalSemaphores.size()),
		signalSemaphores.data(),
	};
	result = vkQueueSubmit(device->getGraphicsQueue(), 1, &submitInfo, nullptr);
	assert(result == VK_SUCCESS);

    // Geometry:
	signalSemaphores = { stageFinishedSemaphores[GEOMETRY] };
	submitInfo = {
		VK_STRUCTURE_TYPE_SUBMIT_INFO,
		nullptr,
		0,
		nullptr,
		nullptr,
		1,
		&graphicsCommands.at(GEOMETRY)[0],
		uint32_t(signalSemaphores.size()),
		signalSemaphores.data(),
	};
	result = vkQueueSubmit(device->getGraphicsQueue(), 1, &submitInfo, nullptr);
	assert(result == VK_SUCCESS);

    // Ssao:
	std::vector<VkSemaphore> waitSemaphores{ stageFinishedSemaphores[GEOMETRY] };
	std::vector<VkPipelineStageFlags> waitStages{ VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT };
	signalSemaphores = { stageFinishedSemaphores[SSAO] };
	submitInfo = {
		VK_STRUCTURE_TYPE_SUBMIT_INFO,
		nullptr,
		uint32_t(waitSemaphores.size()),
		waitSemaphores.data(),
		waitStages.data(),
		1,
		&graphicsCommands.at(SSAO)[0],
		uint32_t(signalSemaphores.size()),
		signalSemaphores.data(),
	};
	result = vkQueueSubmit(device->getGraphicsQueue(), 1, &submitInfo, nullptr);
	assert(result == VK_SUCCESS);

    // Ssao blur:
	waitSemaphores = { stageFinishedSemaphores[SSAO] };
	waitStages = { VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT };
	signalSemaphores = { stageFinishedSemaphores[SSAO_BLUR] };
	submitInfo = {
		VK_STRUCTURE_TYPE_SUBMIT_INFO,
		nullptr,
		uint32_t(waitSemaphores.size()),
		waitSemaphores.data(),
		waitStages.data(),
		1,
		&graphicsCommands.at(SSAO_BLUR)[0],
		uint32_t(signalSemaphores.size()),
		signalSemaphores.data(),
	};
	result = vkQueueSubmit(device->getGraphicsQueue(), 1, &submitInfo, nullptr);
	assert(result == VK_SUCCESS);

    // Lighting:
	waitSemaphores = { stageFinishedSemaphores[SSAO_BLUR], stageFinishedSemaphores[DEPTH] };
	waitStages = { VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT };
	signalSemaphores = { stageFinishedSemaphores[LIGHTING] };
	submitInfo = {
		VK_STRUCTURE_TYPE_SUBMIT_INFO,
		nullptr,
		uint32_t(waitSemaphores.size()),
		waitSemaphores.data(),
		waitStages.data(),
		1,
		&graphicsCommands.at(LIGHTING)[0],
		uint32_t(signalSemaphores.size()),
		signalSemaphores.data(),
	};
	result = vkQueueSubmit(device->getGraphicsQueue(), 1, &submitInfo, nullptr);
	assert(result == VK_SUCCESS);

    // Final:
	waitSemaphores = { stageFinishedSemaphores[LIGHTING], imageAvailableSemaphore };
	waitStages = { VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	signalSemaphores = { stageFinishedSemaphores[FINAL] };
	submitInfo = {
		VK_STRUCTURE_TYPE_SUBMIT_INFO,
		nullptr,
		uint32_t(waitSemaphores.size()),
		waitSemaphores.data(),
		waitStages.data(),
		1,
		&graphicsCommands.at(FINAL)[imageIndex],
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
    if (graphicsCommands.empty())
    {
		for (auto[type, renderPass] : renderPasses)
		{
			graphicsCommands.insert({ type, {} });
		}
    }

    const VkCommandPool commandPool = device->getCommandPool();

    for (auto &[type, commandBuffers] : graphicsCommands)
    {
		if (!commandBuffers.empty())
		{
			vkFreeCommandBuffers(device->get(), commandPool, uint32_t(commandBuffers.size()), commandBuffers.data());
		}

		uint32_t size = 1;
        if (type == FINAL)
        {
			size = swapChain->getImageCount();
        }
		commandBuffers.resize(size);

		VkCommandBufferAllocateInfo allocInfo{
		    VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		    nullptr,
		    commandPool,
		    VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		    size,
		};

		VkResult result = vkAllocateCommandBuffers(device->get(), &allocInfo, commandBuffers.data());
		assert(result == VK_SUCCESS);

		VkCommandBufferBeginInfo beginInfo{
			VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			nullptr,
			VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
			nullptr,
		};

        for (uint32_t i = 0; i < size; i++)
        {
			result = vkBeginCommandBuffer(commandBuffers[i], &beginInfo);
			assert(result == VK_SUCCESS);

			recordRenderPassCommands(type, i, renderPasses.at(type)->getRenderCount());

			result = vkEndCommandBuffer(commandBuffers[i]);
			assert(result == VK_SUCCESS);
        }
    }
}

void Engine::recordRenderPassCommands(RenderPassType type, uint32_t commandBufferIndex, uint32_t renderCount)
{
	for (uint32_t i = 0; i < renderCount; i++)
	{
		beginRenderPass(type, commandBufferIndex, commandBufferIndex + i);

		scene->render(graphicsCommands.at(type)[commandBufferIndex], type, i);

		vkCmdEndRenderPass(graphicsCommands.at(type)[commandBufferIndex]);
	}
}

void Engine::beginRenderPass(RenderPassType type, uint32_t commandBufferIndex, uint32_t framebufferIndex)
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

	vkCmdBeginRenderPass(graphicsCommands.at(type)[commandBufferIndex], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
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





