#include <cassert>

#include "RenderPass.h"

// public:

RenderPass::RenderPass(Device *pDevice, SwapChain *pSwapChain)
{
	device = pDevice->device;
	framebuffersExtent = pSwapChain->extent;

	VkFormat depthAttachmentFormat = pDevice->findSupportedFormat(
		DEPTH_FORMATS,
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);

	createRenderPass(pSwapChain->imageFormat, depthAttachmentFormat);

	pDepthImage = createDepthImage(pDevice, pSwapChain->extent, depthAttachmentFormat, 0);
	pShadowImage = createDepthImage(pDevice, pSwapChain->extent, depthAttachmentFormat, VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT);

	createFramebuffers(pSwapChain->imageViews);
}

RenderPass::~RenderPass()
{
	for (int i = 0; i < framebuffers.size(); i++)
	{
		vkDestroyFramebuffer(device, framebuffers[i], nullptr);
	}

	delete(pShadowImage);
	delete(pDepthImage);

	vkDestroyRenderPass(device, renderPass, nullptr);
}

Image * RenderPass::getShadowInputAttachment() const
{
	return pShadowImage;
}

// private:

void RenderPass::createRenderPass(VkFormat colorAttachmentFormat, VkFormat depthAttachmentFormat)
{
	// description of attachments

	VkAttachmentDescription colorAttachment{
		0,									// flags;
		colorAttachmentFormat,				// format;
		VK_SAMPLE_COUNT_1_BIT,				// samples;
		VK_ATTACHMENT_LOAD_OP_CLEAR,		// loadOp;
		VK_ATTACHMENT_STORE_OP_DONT_CARE,	// storeOp;
		VK_ATTACHMENT_LOAD_OP_DONT_CARE,	// stencilLoadOp;
		VK_ATTACHMENT_STORE_OP_DONT_CARE,	// stencilStoreOp;
		VK_IMAGE_LAYOUT_UNDEFINED,			// initialLayout;
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,	// finalLayout;
	};

	VkAttachmentDescription depthAttachment{
		0,													// flags;
		depthAttachmentFormat,								// format;
		VK_SAMPLE_COUNT_1_BIT,								// samples;
		VK_ATTACHMENT_LOAD_OP_CLEAR,						// loadOp;
		VK_ATTACHMENT_STORE_OP_DONT_CARE,					// storeOp;
		VK_ATTACHMENT_LOAD_OP_DONT_CARE,					// stencilLoadOp;
		VK_ATTACHMENT_STORE_OP_DONT_CARE,					// stencilStoreOp;
		VK_IMAGE_LAYOUT_UNDEFINED,							// initialLayout;
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,	// finalLayout;
	};

	VkAttachmentDescription shadowAttachment{
		0,													// flags;
		depthAttachmentFormat,								// format;
		VK_SAMPLE_COUNT_1_BIT,								// samples;
		VK_ATTACHMENT_LOAD_OP_CLEAR,						// loadOp;
		VK_ATTACHMENT_STORE_OP_DONT_CARE,					// storeOp;
		VK_ATTACHMENT_LOAD_OP_DONT_CARE,					// stencilLoadOp;
		VK_ATTACHMENT_STORE_OP_STORE,						// stencilStoreOp;
		VK_IMAGE_LAYOUT_UNDEFINED,							// initialLayout;
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,	// finalLayout;
	};

	std::vector<VkAttachmentDescription> attachments{
		colorAttachment,
		depthAttachment,
		shadowAttachment
	};

	// references to attachments

	VkAttachmentReference colorAttachmentRef{
		0,											// attachment;
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL	// layout;
	};

	VkAttachmentReference depthAttachmentRef{
		1,													// attachment;
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL	// layout;
	};

	VkAttachmentReference shadowAttachmentRef{
		2,													// attachment;
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL	// layout;
	};

	VkAttachmentReference inputAttachmentRef{
		2,											// attachment;
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL	// layout;
	};

	// subpasses:

	VkSubpassDescription shadowSubpass{
		0,									// flags;
		VK_PIPELINE_BIND_POINT_GRAPHICS,	// pipelineBindPoint;
		0,									// inputAttachmentCount;
		nullptr,							// pInputAttachmentReferences;
		1,									// colorAttachmentCount;
		&colorAttachmentRef,				// pColorAttachmentReferences;
		nullptr,							// pResolveAttachmentReference;
		&shadowAttachmentRef,				// pDepthStencilAttachmentReference;
		0,									// preserveAttachmentCount;
		nullptr								// pPreserveAttachments;
	};

	VkSubpassDescription mainSubpass{
		0,									// flags;
		VK_PIPELINE_BIND_POINT_GRAPHICS,	// pipelineBindPoint;
		1,									// inputAttachmentCount;
		&inputAttachmentRef,				// pInputAttachmentReferences;
		1,									// colorAttachmentCount;
		&colorAttachmentRef,				// pColorAttachmentReferences;
		nullptr,							// pResolveAttachmentReference;
		&depthAttachmentRef,				// pDepthStencilAttachmentReference;
		0,									// preserveAttachmentCount;
		nullptr								// pPreserveAttachments;
	};

	std::vector<VkSubpassDescription> subpasses{
		shadowSubpass,
		mainSubpass
	};

	// subpass dependecies:

	VkSubpassDependency inputDependency{
		VK_SUBPASS_EXTERNAL,																		// srcSubpass;
		0,																							// dstSubpass;
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,														// srcStageMask;
		VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,													// dstStageMask;
		VK_ACCESS_MEMORY_READ_BIT,																	// srcAccessMask;
		VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,	// dstAccessMask;
		VK_DEPENDENCY_BY_REGION_BIT,																// dependencyFlags;
	};

	VkSubpassDependency dependecy{
		0,																							// srcSubpass;
		1,																							// dstSubpass;
		VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,													// srcStageMask;
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,														// dstStageMask;
		VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,	// srcAccessMask;
		VK_ACCESS_SHADER_READ_BIT,																	// dstAccessMask;
		VK_DEPENDENCY_BY_REGION_BIT,																// dependencyFlags;
	};

	VkSubpassDependency outputDependency{
		0,																			// srcSubpass;
		1,																			// dstSubpass;
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,								// srcStageMask;
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,										// dstStageMask;
		VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,	// srcAccessMask;
		VK_ACCESS_MEMORY_READ_BIT,													// dstAccessMask;
		VK_DEPENDENCY_BY_REGION_BIT,												// dependencyFlags;
	};

	std::vector<VkSubpassDependency> dependecies{
		inputDependency,
		dependecy,
		outputDependency
	};

	// render pass:

	VkRenderPassCreateInfo createInfo{
		VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,	// sType;
		nullptr,									// pNext;
		0,											// flags;
		attachments.size(),							// attachmentCount;
		attachments.data(),							// pAttachments;
		subpasses.size(),							// subpassCount;
		subpasses.data(),							// pSubpasses;
		dependecies.size(),							// dependencyCount;
		dependecies.data(),							// pDependencies;
	};

	VkResult result = vkCreateRenderPass(device, &createInfo, nullptr, &renderPass);
	assert(result == VK_SUCCESS);
}

Image* RenderPass::createDepthImage(Device *pDevice, VkExtent2D depthImageExtent, VkFormat depthImagetFormat, VkImageUsageFlags usage)
{
	VkExtent3D extent{
		depthImageExtent.width,
		depthImageExtent.height,
		1
	};

	Image *pImage = new Image(
		pDevice,
		extent,
		0,
		1,
		depthImagetFormat,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | usage,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		1
	);

	VkImageSubresourceRange subresourceRange{
		VK_IMAGE_ASPECT_DEPTH_BIT,	// aspectMask;
		0,							// baseMipLevel;
		1,							// levelCount;
		0,							// baseArrayLayer;
		1,							// layerCount;
	};

	pImage->createImageView(subresourceRange, VK_IMAGE_VIEW_TYPE_2D);

	pImage->transitLayout(
		pDevice,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		subresourceRange
	);

	return pImage;
}

void RenderPass::createFramebuffers(std::vector<VkImageView> swapChainImageViews)
{
	framebuffers.resize(swapChainImageViews.size());

	VkExtent3D extent = pDepthImage->extent;

	for (int i = 0; i < swapChainImageViews.size(); i++)
	{
		std::vector<VkImageView> attachments{
			swapChainImageViews[i],
			pDepthImage->view,
			pShadowImage->view
		};

		VkFramebufferCreateInfo createInfo{
			VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,	// sType;
			nullptr,									// pNext;
			0,											// flags;
			renderPass,									// renderPass;
			attachments.size(),							// attachmentCount;
			attachments.data(),							// pAttachments;
			extent.width,								// width;
			extent.height,								// height;
			1,											// layers;
		};

		VkResult result = vkCreateFramebuffer(device, &createInfo, nullptr, &framebuffers[i]);
		assert(result == VK_SUCCESS);
	}
}

