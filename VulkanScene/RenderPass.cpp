#include "RenderPass.h"

// public:

RenderPass::~RenderPass()
{
	cleanup();
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
	cleanup();
	extent = newExtent;
	create();
}

RenderPass::RenderPass(Device *pDevice, VkExtent2D extent)
{
    this->pDevice = pDevice;
    this->extent = extent;

    depthAttachmentFormat = pDevice->findSupportedFormat(
        DEPTH_FORMATS,
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}

void RenderPass::cleanup()
{
	for (VkFramebuffer framebuffer : framebuffers)
	{
		vkDestroyFramebuffer(pDevice->device, framebuffer, nullptr);
	}
	vkDestroyRenderPass(pDevice->device, renderPass, nullptr);
}

