#include <set>
#include <iostream>
#include "ShaderModule.h"
#include "AssimpModel.h"
#include "FinalRenderPass.h"
#include "DepthRenderPass.h"
#include "GeometryRenderPass.h"
#include "LightingRenderPass.h"
#include "SsaoRenderPass.h"

#include "Engine.h"

// public:

Engine::Engine(
    HINSTANCE hInstance,
    HWND hWnd,
    VkExtent2D frameExtent,
    Settings settings
)
{
    std::cout << "Loading engine..." << std::endl;

	const std::string VALIDATION_LAYER = "VK_LAYER_LUNARG_standard_validation";
	std::vector<const char*> requiredLayers;
#ifdef _DEBUG 
	requiredLayers.push_back(VALIDATION_LAYER.c_str());
#endif

	const std::vector<const char *> EXTENTIONS{
		VK_KHR_SURFACE_EXTENSION_NAME,
		"VK_KHR_win32_surface"
	};

	this->ssaoEnabled = settings.ssaoEnabled;

	pInstance = new Instance(requiredLayers, EXTENTIONS);
	pSurface = new Surface(pInstance->getInstance(), hInstance, hWnd);
	pDevice = new Device(pInstance->getInstance(), pSurface->getSurface(), requiredLayers, settings.sampleCount);
	pSwapChain = new SwapChain(pDevice, pSurface->getSurface(), frameExtent);

	createRenderPasses(settings.shadowsDim);

	pScene = new Scene(pDevice, pSwapChain->getExtent(), "assets/SceneSunset.json", settings.shadowsDistance, settings.modelsExistence);
	pDescriptorPool = new DescriptorPool(pDevice, pScene->getBufferCount(), pScene->getTextureCount(), pScene->getDescriptorSetCount());

	pScene->prepareSceneRendering(pDescriptorPool, renderPasses);

	initGraphicsCommands();

	createSemaphore(pDevice->device, imageAvailable);
	createSemaphore(pDevice->device, renderingFinished);
}

Engine::~Engine()
{
	vkDeviceWaitIdle(pDevice->device);

	vkDestroySemaphore(pDevice->device, imageAvailable, nullptr);
	vkDestroySemaphore(pDevice->device, renderingFinished, nullptr);

	delete(pScene);
	delete(pDescriptorPool);
    for (auto renderPass : renderPasses)
    {
		delete renderPass.second;
    }
	delete(pSwapChain);
	delete(pDevice);
	delete(pSurface);
	delete(pInstance);
}

void Engine::drawFrame()
{
	pScene->updateScene();

	if (minimized)
	{
		return;
	}

	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(
		pDevice->device,
		pSwapChain->getSwapchain(),
		(std::numeric_limits<uint64_t>::max)(),
		imageAvailable,
		nullptr,
		&imageIndex
	);
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		resize(pSwapChain->getExtent());
		return;
	}

    assert(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR);

    std::vector<VkSemaphore> waitSemaphores{ imageAvailable };
	std::vector<VkPipelineStageFlags> waitStages{ VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	std::vector<VkSemaphore> signalSemaphores{ renderingFinished };
	VkSubmitInfo submitInfo{
		VK_STRUCTURE_TYPE_SUBMIT_INFO,	// sType;
		nullptr,						// pNext;
		waitSemaphores.size(),			// waitSemaphoreCount;
		waitSemaphores.data(),			// pWaitSemaphores;
		waitStages.data(),				// pWaitDstStageMask;
		1,								// commandBufferCount;
		&graphicCommands[imageIndex],	// pCommandBuffers;
		signalSemaphores.size(),		// signalSemaphoreCount;
		signalSemaphores.data(),		// pSignalSemaphores;
	};

	result = vkQueueSubmit(pDevice->graphicsQueue, 1, &submitInfo, nullptr);
	assert(result == VK_SUCCESS);

	std::vector<VkSwapchainKHR> swapChains{ pSwapChain->getSwapchain() };
	VkPresentInfoKHR presentInfo{
		VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,	// sType;
		nullptr,							// pNext;
		signalSemaphores.size(),			// waitSemaphoreCount;
		signalSemaphores.data(),			// pWaitSemaphores;
		swapChains.size(),					// swapchainCount;
		swapChains.data(),					// pSwapchains;
		&imageIndex,						// pImageIndices;
		nullptr,							// pResults;
	};

	result = vkQueuePresentKHR(pDevice->presentQueue, &presentInfo);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
	{
		resize(pSwapChain->getExtent());
	}
	else
	{
		assert(result == VK_SUCCESS);
	}
}

void Engine::resize(VkExtent2D newExtent)
{
	if (minimized)
	{
		return;
	}

	vkDeviceWaitIdle(pDevice->device);

	pSwapChain->recreate(newExtent);

    for (auto renderPass : renderPasses)
    {
        if (renderPass.first != DEPTH)
        {
			renderPass.second->recreate(pSwapChain->getExtent());
        }
    }

	pScene->updateDescriptorSets(pDescriptorPool, renderPasses);
	pScene->resizeExtent(pSwapChain->getExtent());

	initGraphicsCommands();
}

// private:

void Engine::createRenderPasses(uint32_t shadowsDim)
{
	const VkExtent2D depthMapExtent = { shadowsDim, shadowsDim };

	renderPasses.insert({ DEPTH, new DepthRenderPass(pDevice, depthMapExtent) });
	renderPasses.insert({ GEOMETRY, new GeometryRenderPass(pDevice, pSwapChain->getExtent()) });
	renderPasses.insert({ SSAO, new SsaoRenderPass(pDevice, pSwapChain->getExtent()) });
	renderPasses.insert({ SSAO_BLUR, new SsaoRenderPass(pDevice, pSwapChain->getExtent()) });
	renderPasses.insert({ LIGHTING, new LightingRenderPass(pDevice, pSwapChain) });
	renderPasses.insert({ FINAL, new FinalRenderPass(pDevice, pSwapChain) });

    for (auto renderPass : renderPasses)
    {
        if (renderPass.first == FINAL)
        {
			dynamic_cast<FinalRenderPass*>(renderPass.second)->saveRenderPasses(
				dynamic_cast<GeometryRenderPass*>(renderPasses.at(GEOMETRY)),
				dynamic_cast<LightingRenderPass*>(renderPasses.at(LIGHTING))
			);
        }
		renderPass.second->create();
    }
}

void Engine::initGraphicsCommands()
{
	// return old command buffers to pool
	if (!graphicCommands.empty())
	{
		vkFreeCommandBuffers(pDevice->device, pDevice->commandPool, graphicCommands.size(), graphicCommands.data());
	}

	graphicCommands.resize(pSwapChain->getImageCount());

	VkCommandBufferAllocateInfo allocInfo{
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,	// sType;
		nullptr,										// pNext;
		pDevice->commandPool,							// commandPool;
		VK_COMMAND_BUFFER_LEVEL_PRIMARY,				// level;
		graphicCommands.size(),							// commandBufferCount;
	};

	VkResult result = vkAllocateCommandBuffers(pDevice->device, &allocInfo, graphicCommands.data());
	assert(result == VK_SUCCESS);

	for (size_t i = 0; i < graphicCommands.size(); i++)
	{
		VkCommandBufferBeginInfo beginInfo{
			VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,	// sType;
			nullptr,										// pNext;
			VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,	// flags;
			nullptr,										// pInheritanceInfo;
		};

		result = vkBeginCommandBuffer(graphicCommands[i], &beginInfo);
		assert(result == VK_SUCCESS);

		recordRenderPassCommands(graphicCommands[i], DEPTH, 0);
		recordRenderPassCommands(graphicCommands[i], GEOMETRY, 0);
        if (ssaoEnabled)
        {
			recordRenderPassCommands(graphicCommands[i], SSAO, 0);
			recordRenderPassCommands(graphicCommands[i], SSAO_BLUR, 0);
        }
		recordRenderPassCommands(graphicCommands[i], LIGHTING, 0);
		recordRenderPassCommands(graphicCommands[i], FINAL, i);

		result = vkEndCommandBuffer(graphicCommands[i]);

		assert(result == VK_SUCCESS);
	}
}

void Engine::beginRenderPass(VkCommandBuffer commandBuffer, RenderPassType type, uint32_t framebufferIndex)
{
	VkRect2D renderArea{
		{ 0, 0 },                           // offset
		renderPasses.at(type)->getExtent()  // extent
	};

	std::vector<VkClearValue> clearValues = renderPasses.at(type)->getClearValues();

	VkRenderPassBeginInfo renderPassBeginInfo{
	    VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,                   // sType;
	    nullptr,                                                    // pNext;
	    renderPasses.at(type)->getRenderPass(),                     // renderPass;
	    renderPasses.at(type)->getFramebuffers()[framebufferIndex], // framebuffer;
	    renderArea,                                                 // renderArea;
	    uint32_t(clearValues.size()),                               // clearValueCount;
		clearValues.data()                                          // pClearValues;
	};

	vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void Engine::recordRenderPassCommands(VkCommandBuffer commandBuffer, RenderPassType type, uint32_t framebufferIndex)
{
	beginRenderPass(commandBuffer, type, framebufferIndex);

	pScene->render(commandBuffer, type);

	vkCmdEndRenderPass(commandBuffer);
}

void Engine::createSemaphore(VkDevice device, VkSemaphore& semaphore)
{
	if (semaphore != nullptr)
	{
		vkDestroySemaphore(device, semaphore, nullptr);
	}

	VkSemaphoreCreateInfo createInfo{
		VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,	// sType;
		nullptr,									// pNext;
		0,											// flags;
	};

	VkResult result = vkCreateSemaphore(device, &createInfo, nullptr, &semaphore);
	assert(result == VK_SUCCESS);
}





