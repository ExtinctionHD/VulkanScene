﻿#include <cassert>
#include "PssmKernel.h"

#include "DepthRenderPass.h"

// public:

DepthRenderPass::DepthRenderPass(Device *device, VkExtent2D attachmentExtent)
    : RenderPass(device, attachmentExtent, VK_SAMPLE_COUNT_1_BIT)
{
}

DepthRenderPass::~DepthRenderPass()
{
	destroyCascadeViews();
}

std::shared_ptr<TextureImage> DepthRenderPass::getDepthTexture() const
{
	return depthTexture;
}

uint32_t DepthRenderPass::getRenderCount() const
{
	return PssmKernel::CASCADE_COUNT;
}

// protected:

void DepthRenderPass::createAttachments()
{
    const VkExtent3D attachmentExtent{
		extent.width,
		extent.height,
		1
	};

	depthTexture = std::make_shared<TextureImage>(
		device,
		attachmentExtent,
		0,
		sampleCount,
        1,
		depthAttachmentFormat,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        PssmKernel::CASCADE_COUNT,
        false,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		VK_IMAGE_ASPECT_DEPTH_BIT,
        VK_FILTER_LINEAR,
		VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);

	attachments = { depthTexture };

    if (!cascadeViews.empty())
    {
		destroyCascadeViews();
    }

	cascadeViews.resize(PssmKernel::CASCADE_COUNT);
    for (uint32_t i = 0; i < PssmKernel::CASCADE_COUNT; i++)
    {
        const VkImageSubresourceRange subresourceRange{
			VK_IMAGE_ASPECT_DEPTH_BIT,
			0,
			1,
			i,
			1
		};

		cascadeViews[i] = depthTexture->createImageView(subresourceRange, VK_IMAGE_VIEW_TYPE_2D);
    }
}

void DepthRenderPass::createRenderPass()
{
	VkAttachmentDescription depthAttachmentDesc{
	    0,							
        depthTexture->getFormat(),                
        depthTexture->getSampleCount(),		
        VK_ATTACHMENT_LOAD_OP_CLEAR,		
        VK_ATTACHMENT_STORE_OP_STORE,	
        VK_ATTACHMENT_LOAD_OP_DONT_CARE,	
        VK_ATTACHMENT_STORE_OP_DONT_CARE,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
	};

	// references to attachments

	VkAttachmentReference depthAttachmentRef{
		0,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
	};

	// subpass and it dependencies (contain references)

	VkSubpassDescription subpass{
		0,						
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		0,								
		nullptr,						
		0,								
		nullptr,				        
		nullptr,						
		&depthAttachmentRef,	
		0,								
		nullptr							
	};

    const VkSubpassDependency inputDependency{
		VK_SUBPASS_EXTERNAL,                         
		0,                                           
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,        
		VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,   
		0,                                           
		VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
		VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
		VK_DEPENDENCY_BY_REGION_BIT,                 
	};

    const VkSubpassDependency outputDependency{
		0,                                 
		VK_SUBPASS_EXTERNAL,                         
		VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,   
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,       
		VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
		VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
		VK_ACCESS_SHADER_READ_BIT,                   
		VK_DEPENDENCY_BY_REGION_BIT,                 
	};

	std::vector<VkSubpassDependency> dependencies{
		inputDependency,
		outputDependency
	};

	// render pass (contain descriptions)

	VkRenderPassCreateInfo createInfo{
		VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		nullptr,									
		0,											
		1,							              
		&depthAttachmentDesc,						
		1,											
		&subpass,									
		uint32_t(dependencies.size()),
		dependencies.data(),						
	};

    const VkResult result = vkCreateRenderPass(device->get(), &createInfo, nullptr, &renderPass);
	assert(result == VK_SUCCESS);
}

void DepthRenderPass::createFramebuffers()
{
    for (const auto &view : cascadeViews)
    {
		addFramebuffer({ view });
    }
}

// private:

void DepthRenderPass::destroyCascadeViews()
{
	for (auto &view : cascadeViews)
	{
		vkDestroyImageView(device->get(), view, nullptr);
	}
}
