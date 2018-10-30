#include "DepthRenderPass.h"
#include <cassert>

// public:

DepthRenderPass::DepthRenderPass(Device *pDevice, VkExtent2D textureExtent) : RenderPass(pDevice, textureExtent)
{
}

DepthRenderPass::~DepthRenderPass()
{
	delete pDepthMap;
}

TextureImage * DepthRenderPass::getDepthMap() const
{
	return pDepthMap;
}

// protected:

void DepthRenderPass::createRenderPass()
{
	VkAttachmentDescription depthAttachment{
		   0,													// flags;
		   depthAttachmentFormat,								// format;
		   VK_SAMPLE_COUNT_1_BIT,							    // samples;
		   VK_ATTACHMENT_LOAD_OP_CLEAR,						    // loadOp;
		   VK_ATTACHMENT_STORE_OP_STORE,					    // storeOp;
		   VK_ATTACHMENT_LOAD_OP_DONT_CARE,					    // stencilLoadOp;
		   VK_ATTACHMENT_STORE_OP_DONT_CARE,					// stencilStoreOp;
		   VK_IMAGE_LAYOUT_UNDEFINED,							// initialLayout;
		   VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,	// finalLayout;
	};

	// references to attachments

	VkAttachmentReference depthAttachmentRef{
		0,													// attachment;
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
		&depthAttachment,							// pAttachments;
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
	VkExtent3D textureExtent{
		extent.width,
		extent.height,
		1
	};

	delete pDepthMap;

	pDepthMap = new TextureImage(
		pDevice,
	    textureExtent,
	    0,
        VK_SAMPLE_COUNT_1_BIT,
	    depthAttachmentFormat,
	    VK_IMAGE_TILING_OPTIMAL,
	    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
	    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
	    VK_IMAGE_ASPECT_DEPTH_BIT,
	    VK_IMAGE_VIEW_TYPE_2D,
	    1
	);

	VkFramebuffer framebuffer;
	VkFramebufferCreateInfo createInfo{
		VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,	// sType;
		nullptr,									// pNext;
		0,											// flags;
		renderPass,									// renderPass;
		1,							                // attachmentCount;
		&pDepthMap->view,						    // pAttachments;
		extent.width,								// width;
		extent.height,								// height;
		1,											// layers;
	};

	VkResult result = vkCreateFramebuffer(pDevice->device, &createInfo, nullptr, &framebuffer);
	assert(result == VK_SUCCESS);

	framebuffers.push_back(framebuffer);
}

// private:
