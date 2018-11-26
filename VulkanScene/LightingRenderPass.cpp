#include "LightingRenderPass.h"
#include <cassert>


LightingRenderPass::LightingRenderPass(Device *pDevice, SwapChain *pSwapChain) : RenderPass(pDevice, pSwapChain->getExtent())
{
	colorAttachmentFormat = pSwapChain->getImageFormat();
	sampleCount = pDevice->getSampleCount();
}

uint32_t LightingRenderPass::getColorAttachmentCount() const
{
	return 1;
}

Image * LightingRenderPass::getColorImage() const
{
	return pColorImage;
}

void LightingRenderPass::createAttachments()
{
	VkExtent3D attachmentExtent{
		extent.width,
		extent.height,
		1
	};

	pColorImage = new Image(
		pDevice,
		attachmentExtent,
		0,
		pDevice->getSampleCount(),
		1,
		colorAttachmentFormat,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		1
	);
	attachments.push_back(pColorImage);

	VkImageSubresourceRange subresourceRange{
		VK_IMAGE_ASPECT_COLOR_BIT,	// aspectMask;
		0,							// baseMipLevel;
		1,							// levelCount;
		0,							// baseArrayLayer;
		1,							// layerCount;
	};

	pColorImage->createImageView(subresourceRange, VK_IMAGE_VIEW_TYPE_2D);

	pColorImage->transitLayout(
		pDevice,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		subresourceRange
	);
}

void LightingRenderPass::createRenderPass()
{
	// description of attachments

	VkAttachmentDescription colorAttachmentDesc{
		0,									        // flags;
		pColorImage->format,		                // format;
		pColorImage->getSampleCount(),			    // samples;
		VK_ATTACHMENT_LOAD_OP_CLEAR,		        // loadOp;
		VK_ATTACHMENT_STORE_OP_STORE,		        // storeOp;
		VK_ATTACHMENT_LOAD_OP_DONT_CARE,	        // stencilLoadOp;
		VK_ATTACHMENT_STORE_OP_DONT_CARE,	        // stencilStoreOp;
		VK_IMAGE_LAYOUT_UNDEFINED,			        // initialLayout;
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,   // finalLayout;
	};

	std::vector<VkAttachmentDescription> attachmentDescriptions{
		colorAttachmentDesc
	};

	// references to attachments

	VkAttachmentReference colorAttachmentRef{
		0,											// attachment;
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL	// layout;
	};

	// subpass and it dependencies (contain references)

	VkSubpassDescription subpass{
		0,									// flags;
		VK_PIPELINE_BIND_POINT_GRAPHICS,	// pipelineBindPoint;
		0,									// inputAttachmentCount;
		nullptr,							// pInputAttachmentReferences;
		1,									// colorAttachmentCount;
		&colorAttachmentRef,				// pColorAttachmentReferences;
		nullptr,			                // pResolveAttachmentReference;
		nullptr,				            // pDepthStencilAttachmentReference;
		0,									// preserveAttachmentCount;
		nullptr								// pPreserveAttachments;
	};

	VkSubpassDependency inputDependency{
		VK_SUBPASS_EXTERNAL,							// srcSubpass;
		0,												// dstSubpass;
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,			// srcStageMask;
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,	// dstStageMask;
		VK_ACCESS_MEMORY_READ_BIT,						// srcAccessMask;
		VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,	        // dstAccessMask;
		VK_DEPENDENCY_BY_REGION_BIT,                    // dependencyFlags;
	};

	VkSubpassDependency outputDependency{
		0,												// srcSubpass;
		VK_SUBPASS_EXTERNAL,							// dstSubpass;
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,	// srcStageMask;
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,			// dstStageMask;
		VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,	        // srcAccessMask;
		VK_ACCESS_MEMORY_READ_BIT,						// dstAccessMask;
		VK_DEPENDENCY_BY_REGION_BIT,					// dependencyFlags;
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
		attachmentDescriptions.size(),				// attachmentCount;
		attachmentDescriptions.data(),				// pAttachments;
		1,											// subpassCount;
		&subpass,									// pSubpasses;
		dependencies.size(),						// dependencyCount;
		dependencies.data(),						// pDependencies;
	};

	VkResult result = vkCreateRenderPass(pDevice->device, &createInfo, nullptr, &renderPass);
	assert(result == VK_SUCCESS);
}

void LightingRenderPass::createFramebuffers()
{
	VkFramebuffer framebuffer;

	std::vector<VkImageView> imageViews{
		pColorImage->view,
	};

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

	VkResult result = vkCreateFramebuffer(pDevice->device, &createInfo, nullptr, &framebuffer);
	assert(result == VK_SUCCESS);

	framebuffers.push_back(framebuffer);
}

