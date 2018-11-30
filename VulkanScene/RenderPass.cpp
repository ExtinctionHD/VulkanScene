#include "RenderPass.h"
#include <cassert>

// public:

RenderPass::~RenderPass()
{
	cleanup();
}

VkRenderPass RenderPass::getRenderPass() const
{
	return renderPass;
}

VkSampleCountFlagBits RenderPass::getSampleCount() const
{
	return sampleCount;
}

std::vector<VkFramebuffer> RenderPass::getFramebuffers() const
{
	return framebuffers;
}

VkExtent2D RenderPass::getExtent() const
{
	return extent;
}

uint32_t RenderPass::getColorAttachmentCount() const
{
	return 1;   // default color attachment count without MRT
}

std::vector<VkClearValue> RenderPass::getClearValues() const
{
	std::vector<VkClearValue> clearValues;
    for(Image *pImage : attachments)
    {
		VkClearValue clearValue{};

        if (pImage->format == depthAttachmentFormat)
        {
			clearValue.depthStencil = { 1.0f, 0 };
        }
		else
		{
			clearValue.color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
		}

		clearValues.push_back(clearValue);
    }

	return clearValues;
}

void RenderPass::create()
{
	createAttachments();
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

void RenderPass::addFramebuffer(std::vector<VkImageView> imageViews)
{

	VkFramebufferCreateInfo createInfo{
		VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,	// sType;
		nullptr,									// pNext;
		0,											// flags;
		renderPass,									// renderPass;
		imageViews.size(),							// attachmentCount;
		imageViews.data(),							// pAttachments;
		extent.width,								// width;
		extent.height,								// height;
		1,											// layers;
	};

	VkFramebuffer framebuffer;

	VkResult result = vkCreateFramebuffer(pDevice->device, &createInfo, nullptr, &framebuffer);
	assert(result == VK_SUCCESS);

	framebuffers.push_back(framebuffer);
}

void RenderPass::cleanup()
{
    for (Image *pImage : attachments)
    {
		delete pImage;
    }
	attachments.clear();

	for (VkFramebuffer framebuffer : framebuffers)
	{
		vkDestroyFramebuffer(pDevice->device, framebuffer, nullptr);
	}
	framebuffers.clear();

	vkDestroyRenderPass(pDevice->device, renderPass, nullptr);
}

