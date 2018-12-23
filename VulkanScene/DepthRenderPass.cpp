﻿#include "DepthRenderPass.h"
#include <cassert>

// public:

DepthRenderPass::DepthRenderPass(Device *pDevice, VkExtent2D attachmentExtent) : RenderPass(pDevice, attachmentExtent)
{
	sampleCount = VK_SAMPLE_COUNT_1_BIT;
}

uint32_t DepthRenderPass::getColorAttachmentCount() const
{
	return 0;
}

TextureImage * DepthRenderPass::getDepthMap() const
{
	return pDepthMap;
}

// protected:

void DepthRenderPass::createAttachments()
{
	VkExtent3D attachmentExtent{
		extent.width,
		extent.height,
		1
	};

	pDepthMap = new TextureImage(
		pDevice,
		attachmentExtent,
		0,
		sampleCount,
		depthAttachmentFormat,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		VK_IMAGE_ASPECT_DEPTH_BIT,
		VK_IMAGE_VIEW_TYPE_2D,
		1,
        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER
	);
	attachments.push_back(pDepthMap);
}

void DepthRenderPass::createRenderPass()
{
	VkAttachmentDescription depthAttachmentDesc{
		   0,													// flags;
		   pDepthMap->format,                                   // format;
		   pDepthMap->getSampleCount(),							// samples;
		   VK_ATTACHMENT_LOAD_OP_CLEAR,						    // loadOp;
		   VK_ATTACHMENT_STORE_OP_STORE,					    // storeOp;
		   VK_ATTACHMENT_LOAD_OP_DONT_CARE,					    // stencilLoadOp;
		   VK_ATTACHMENT_STORE_OP_DONT_CARE,					// stencilStoreOp;
		   VK_IMAGE_LAYOUT_UNDEFINED,	// initialLayout;
		   VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,	// finalLayout;
	};

	// references to attachments

	VkAttachmentReference depthAttachmentRef{
		0,										// attachment;
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL	// layout;
	};

	// subpass and it dependencies (contain references)

	VkSubpassDescription subpass{
		0,									// flags;
		VK_PIPELINE_BIND_POINT_GRAPHICS,	// pipelineBindPoint;
		0,									// inputAttachmentCount;
		nullptr,							// pInputAttachmentReferences;
		0,									// colorAttachmentCount;
		nullptr,				            // pColorAttachmentReferences;
		nullptr,							// pResolveAttachmentReference;
		&depthAttachmentRef,				// pDepthStencilAttachmentReference;
		0,									// preserveAttachmentCount;
		nullptr								// pPreserveAttachments;
	};

	VkSubpassDependency inputDependency{
		VK_SUBPASS_EXTERNAL,                            // srcSubpass;
		0,                                              // dstSubpass;
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,           // srcStageMask;
		VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,      // dstStageMask;
		0,                                              // srcAccessMask;
		VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | 
		VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,   // dstAccessMask;
		VK_DEPENDENCY_BY_REGION_BIT,                    // dependencyFlags;
	};

	VkSubpassDependency outputDependency{
		0,                                              // srcSubpass;
		VK_SUBPASS_EXTERNAL,                            // dstSubpass;
		VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,      // srcStageMask;
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,          // dstStageMask;
		VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | 
		VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,   // srcAccessMask;
		VK_ACCESS_SHADER_READ_BIT,                      // dstAccessMask;
		VK_DEPENDENCY_BY_REGION_BIT,                    // dependencyFlags;
	};

	std::vector<VkSubpassDependency> dependencies{
		inputDependency,
		outputDependency
	};

	// render pass (contain descriptions)

	VkRenderPassCreateInfo createInfo{
		VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,	// sType;
		nullptr,									// pNext;
		0,											// flags;
		1,							                // attachmentCount;
		&depthAttachmentDesc,						// pAttachments;
		1,											// subpassCount;
		&subpass,									// pSubpasses;
		dependencies.size(),						// dependencyCount;
		dependencies.data(),						// pDependencies;
	};

	VkResult result = vkCreateRenderPass(pDevice->device, &createInfo, nullptr, &renderPass);
	assert(result == VK_SUCCESS);
}

void DepthRenderPass::createFramebuffers()
{
	addFramebuffer({ pDepthMap->view });
}

// private:
