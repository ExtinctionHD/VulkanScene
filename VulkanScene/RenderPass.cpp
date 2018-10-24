#include "RenderPass.h"

// public:

RenderPass::~RenderPass()
{
	for (VkFramebuffer framebuffer : framebuffers)
	{
		vkDestroyFramebuffer(pDevice->device, framebuffer, nullptr);
	}

	vkDestroyRenderPass(pDevice->device, renderPass, nullptr);
}

VkRenderPass RenderPass::getRenderPass() const
{
	return renderPass;
}

std::vector<VkFramebuffer> RenderPass::getFramebuffers() const
{
	return framebuffers;
}

VkExtent2D RenderPass::getExtent() const
{
	return extent;
}

void RenderPass::create()
{
	createRenderPass();
	createFramebuffers();
}

// protected:

void RenderPass::recreate(VkExtent2D newExtent)
{
	for (VkFramebuffer framebuffer : framebuffers)
	{
		vkDestroyFramebuffer(pDevice->device, framebuffer, nullptr);
	}

	vkDestroyRenderPass(pDevice->device, renderPass, nullptr);

	extent = newExtent;
	create();
}

RenderPass::RenderPass(Device *pDevice, VkExtent2D extent)
{
	this->pDevice = pDevice;
	this->extent = extent;
}

