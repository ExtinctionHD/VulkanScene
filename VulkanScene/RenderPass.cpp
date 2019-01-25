#include "RenderPass.h"
#include <cassert>

// public:

RenderPass::~RenderPass()
{
	cleanup();
}

VkRenderPass RenderPass::get() const
{
	return renderPass;
}

VkSampleCountFlagBits RenderPass::getSampleCount() const
{
	return sampleCount;
}

uint32_t RenderPass::getRenderCount() const
{
	return 1;
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
	uint32_t count = 0;

	for (const auto &image : attachments)
	{
		if (image->getFormat() != depthAttachmentFormat)
		{
			count++;
		}
	}

	return count;
}

std::vector<VkClearValue> RenderPass::getClearValues() const
{
	std::vector<VkClearValue> clearValues;
    for(const auto &image : attachments)
    {
		VkClearValue clearValue{};

        if (image->getFormat() == depthAttachmentFormat)
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

RenderPass::RenderPass(Device *device, VkExtent2D extent, VkSampleCountFlagBits sampleCount)
{
    this->device = device;
    this->extent = extent;
	this->sampleCount = sampleCount;

    depthAttachmentFormat = device->findSupportedFormat(
        DEPTH_FORMATS,
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

void RenderPass::addFramebuffer(std::vector<VkImageView> imageViews)
{
	VkFramebufferCreateInfo createInfo{
		VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
		nullptr,			
		0,					
		renderPass,			
		uint32_t(imageViews.size()),
		imageViews.data(),	
		extent.width,		
		extent.height,		
		1,					
	};

	VkFramebuffer framebuffer;

    const VkResult result = vkCreateFramebuffer(device->get(), &createInfo, nullptr, &framebuffer);
	assert(result == VK_SUCCESS);

	framebuffers.push_back(framebuffer);
}

void RenderPass::cleanup()
{
	attachments.clear();

	for (auto framebuffer : framebuffers)
	{
		vkDestroyFramebuffer(device->get(), framebuffer, nullptr);
	}
	framebuffers.clear();

	vkDestroyRenderPass(device->get(), renderPass, nullptr);
}

