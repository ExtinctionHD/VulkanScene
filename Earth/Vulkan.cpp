#include <set>
#include <array>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "ShaderModule.h"
#include "AssimpModel.h"

#include "Vulkan.h"

// public:

Vulkan::Vulkan(GLFWwindow *window, VkExtent2D frameExtent)
{
	glfwSetWindowUserPointer(window, this);

	std::vector<const char*> requiredLayers = ENABLE_VALIDATION_LAYERS ?
		VALIDATION_LAYERS : std::vector<const char*>();

	pInstance = new Instance(requiredLayers);

	createSurface(window);

	pDevice = new Device(pInstance->getInstance(), surface, requiredLayers);

	pSwapChain = new SwapChain(pDevice, surface, frameExtent);

	pRenderPass = new RenderPass(pDevice, pSwapChain);

	// const uint32_t binding = 0;

	//// create DS and pipeline for rendering main objects:
	//pMainDS = new DescriptorSet(pDevice);
	//initMainDS();
	/*std::vector<ShaderModule*> mainShaderModules{
		new ShaderModule(pDevice->device, SHADERS_PATHES[Shaders::mainVert], VK_SHADER_STAGE_VERTEX_BIT),
		new ShaderModule(pDevice->device, SHADERS_PATHES[Shaders::mainFrag], VK_SHADER_STAGE_FRAGMENT_BIT),
	};
	pMainPipeline = new GraphicsPipeline(
		pDevice->device, 
		{ pMainDS->layout }, 
		pRenderPass, 
		mainShaderModules,
		Vertex::getBindingDescription(binding),
		Vertex::getAttributeDescriptions(binding)
	);*/

	//// create DS and pipeline for rendering skybox
	//pSkyboxDS = new DescriptorSet(pDevice);
	//initSkyboxDS();

	/*std::vector<ShaderModule*> skyboxShaderModules{
		new ShaderModule(pDevice->device, SHADERS_PATHES[Shaders::skyboxVert], VK_SHADER_STAGE_VERTEX_BIT),
		new ShaderModule(pDevice->device, SHADERS_PATHES[Shaders::skyboxFrag], VK_SHADER_STAGE_FRAGMENT_BIT),
	};
	pSkyboxPipeline = new GraphicsPipeline(
		pDevice->device,
		{ pSkyboxDS->layout },
		pRenderPass,
		skyboxShaderModules,
		Position::getBindingDescription(binding),
		Position::getAttributeDescriptions(binding)
	);*/

	//initGraphicCommands();

	//createSemaphore(pDevice->device, imageAvailable);
	//createSemaphore(pDevice->device, renderingFinished);
}

Vulkan::~Vulkan()
{
	vkDeviceWaitIdle(pDevice->device);

	vkDestroySemaphore(pDevice->device, imageAvailable, nullptr);
	vkDestroySemaphore(pDevice->device, renderingFinished, nullptr);

	delete(pSkyboxPipeline);
	delete(pMainPipeline);
	delete(pScene);
	delete(pSwapChain);
	delete(pDevice);

	vkDestroySurfaceKHR(pInstance->getInstance(), surface, nullptr);

	delete(pInstance);
}

void Vulkan::drawFrame()
{
	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(
		pDevice->device,
		pSwapChain->swapChain,
		(std::numeric_limits<uint64_t>::max)(),
		imageAvailable,
		VK_NULL_HANDLE,
		&imageIndex
	);
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		resize(pSwapChain->extent);
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		throw std::runtime_error("Failed to acquire next frame");
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
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to submit graphics commands");
	}

	std::vector<VkSwapchainKHR> swapChains{ pSwapChain->swapChain };
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
		resize(pSwapChain->extent);
		return;
	}
	else if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to present next frame");
	}
}

void Vulkan::resize(VkExtent2D newExtent)
{
	vkDeviceWaitIdle(pDevice->device);

	delete(pRenderPass);
	delete(pSwapChain);

	pSwapChain = new SwapChain(pDevice, surface, newExtent);
	pRenderPass = new RenderPass(pDevice, pSwapChain);
	pMainPipeline->recreate(pRenderPass);
	pSkyboxPipeline->recreate(pRenderPass);

	initGraphicCommands();
}

// private:

void Vulkan::createSurface(GLFWwindow *window)
{
	// glfw library create surface by it self
	VkResult result = glfwCreateWindowSurface(pInstance->getInstance(), window, nullptr, &surface);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create surface");
	}
}

void Vulkan::initGraphicCommands()
{
	// return old command buffers to pool
	if (!graphicCommands.empty())
	{
		vkFreeCommandBuffers(pDevice->device, pDevice->commandPool, graphicCommands.size(), graphicCommands.data());
	}

	graphicCommands.resize(pSwapChain->imageCount);

	VkCommandBufferAllocateInfo allocInfo{
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,	// sType;
		nullptr,										// pNext;
		pDevice->commandPool,							// commandPool;
		VK_COMMAND_BUFFER_LEVEL_PRIMARY,				// level;
		graphicCommands.size(),							// commandBufferCount;
	};

	VkResult result = vkAllocateCommandBuffers(pDevice->device, &allocInfo, graphicCommands.data());
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate graphics command buffers");
	}

	// clear values for each frame
	std::array<VkClearValue, 2> clearValues = {};
	clearValues[0].color = clearColor;
	clearValues[1].depthStencil = { 1, 0 };

	// render area for each frame
	VkRect2D renderArea{
		{ 0, 0 },			// offset
		pSwapChain->extent	// extent
	};

	for (int i = 0; i < graphicCommands.size(); i++)
	{
		VkCommandBufferBeginInfo beginInfo{
			VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,	// sType;
			nullptr,										// pNext;
			VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,	// flags;
			nullptr,										// pInheritanceInfo;
		};

		result = vkBeginCommandBuffer(graphicCommands[i], &beginInfo);
		if (result != VK_SUCCESS)
		{
			std::runtime_error("Failed to begin graphics command buffers");
		}

		VkRenderPassBeginInfo renderPassBeginInfo{
			VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,	// sType;
			nullptr,									// pNext;
			pRenderPass->renderPass,					// renderPass;
			pRenderPass->framebuffers[i],				// framebuffer;
			renderArea,									// renderArea;
			clearValues.size(),							// clearValueCount;
			clearValues.data()							// pClearValues;
		};

		vkCmdBeginRenderPass(graphicCommands[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		// draw earth model

		vkCmdEndRenderPass(graphicCommands[i]);

		result = vkEndCommandBuffer(graphicCommands[i]);
		if (result != VK_SUCCESS)
		{
			std::runtime_error("Failed to end graphics command buffers");
		}
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
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create semaphore");
	}
}





