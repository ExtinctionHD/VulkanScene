#include <set>
#include <array>
#include "ShaderModule.h"
#include "AssimpModel.h"

#include "Vulkan.h"
#include "FinalRenderPass.h"

// public:

Vulkan::Vulkan(HINSTANCE hInstance, HWND hWnd, VkExtent2D frameExtent)
{
	std::vector<const char*> requiredLayers = ENABLE_VALIDATION_LAYERS ?
		VALIDATION_LAYERS : std::vector<const char*>();

	pInstance = new Instance(requiredLayers, EXTENTIONS);
	pSurface = new Surface(pInstance->getInstance(),hInstance, hWnd);
	pDevice = new Device(pInstance->getInstance(), pSurface->getSurface(), requiredLayers);
	pSwapChain = new SwapChain(pDevice, pSurface->getSurface(), frameExtent);

	createRenderPasses();

	pScene = new Scene(pDevice, pSwapChain->getExtent());
	pDescriptorPool = new DescriptorPool(pDevice, pScene->getBufferCount(), pScene->getTextureCount(), pScene->getDescriptorSetCount());

	pScene->initDescriptorSets(pDescriptorPool);
	pScene->initPipelines(renderPasses.at(final));

	initGraphicsCommands();

	createSemaphore(pDevice->device, imageAvailable);
	createSemaphore(pDevice->device, renderingFinished);
}

Vulkan::~Vulkan()
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

void Vulkan::drawFrame()
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
		VK_NULL_HANDLE,
		&imageIndex
	);
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		resize(pSwapChain->getExtent());
		return;
	}
	else
	{
		assert(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR);
	}

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

	result = vkQueueSubmit(pDevice->graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
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
		return;
	}
	else
	{
		assert(result == VK_SUCCESS);
	}
}

void Vulkan::resize(VkExtent2D newExtent)
{
	if (minimized)
	{
		return;
	}

	vkDeviceWaitIdle(pDevice->device);

	pSwapChain->recreate(newExtent);
	renderPasses.at(final)->recreate(pSwapChain->getExtent());
	pScene->resizeExtent(pSwapChain->getExtent());

	initGraphicsCommands();
}

void Vulkan::keyDownCallback(int key)
{
	pScene->getController()->keyDownCallback(key);
}

void Vulkan::keyUpCallback(int key)
{
	pScene->getController()->keyUpCallback(key);
}

// private:

void Vulkan::createRenderPasses()
{
    auto pRenderPass = new FinalRenderPass(pDevice, pSwapChain);
	pRenderPass->create();

	renderPasses.insert({ final, pRenderPass });
}

void Vulkan::initGraphicsCommands()
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

	// clear values for each frame
	std::array<VkClearValue, 2> clearValues = {};
	clearValues[0].color = CLEAR_COLOR;
	clearValues[1].depthStencil = { 1, 0 };

	// render area for each frame
	VkRect2D renderArea{
		{ 0, 0 },			// offset
		pSwapChain->getExtent()	// extent
	};

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

		VkRenderPassBeginInfo renderPassBeginInfo{
			VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,	// sType;
			nullptr,									// pNext;
			renderPasses.at(final)->getRenderPass(),			// renderPass;
			renderPasses.at(final)->getFramebuffers()[i],		// framebuffer;
			renderArea,									// renderArea;
			clearValues.size(),							// clearValueCount;
			clearValues.data()							// pClearValues;
		};

		vkCmdBeginRenderPass(graphicCommands[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		// draw each model
		pScene->draw(graphicCommands[i]);

		vkCmdEndRenderPass(graphicCommands[i]);

		result = vkEndCommandBuffer(graphicCommands[i]);
		assert(result == VK_SUCCESS);
	}
}

void Vulkan::createSemaphore(VkDevice device, VkSemaphore& semaphore)
{
	if (semaphore != VK_NULL_HANDLE)
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





