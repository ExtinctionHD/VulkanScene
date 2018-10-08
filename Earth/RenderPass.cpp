#include "RenderPass.h"

// public:

RenderPass::RenderPass(Device *pDevice, SwapChain *pSwapChain)
{
	device = pDevice->device;
	attachmentsExtent = pSwapChain->extent;

	VkFormat depthAttachmentFormat = pDevice->findSupportedFormat(
		DEPTH_FORMATS,
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);

	createRenderPass(pSwapChain->imageFormat, depthAttachmentFormat);

	createDepthResources(pDevice, pSwapChain->extent, depthAttachmentFormat);

	createFramebuffers(pSwapChain->imageViews);
}

RenderPass::~RenderPass()
{
	for (int i = 0; i < framebuffers.size(); i++)
	{
		vkDestroyFramebuffer(device, framebuffers[i], nullptr);
	}

	delete(pDepthImage);

	vkDestroyRenderPass(device, renderPass, nullptr);
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
		VK_ATTACHMENT_STORE_OP_STORE,		// storeOp;
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

	std::vector<VkAttachmentDescription> attachments{
		colorAttachment,
		depthAttachment
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

	// subpass and it dependecies (contain references)

	VkSubpassDescription subpass{
		0,									// flags;
		VK_PIPELINE_BIND_POINT_GRAPHICS,	// pipelineBindPoint;
		0,									// inputAttachmentCount;
		nullptr,							// pInputAttachmentReferences;
		1,									// colorAttachmentCount;
		&colorAttachmentRef,				// pColorAttachmentReferences;
		nullptr,							// pResolveAttachmentReference;
		&depthAttachmentRef,				// pDepthStencilAttachmentReference;
		0,									// preserveAttachmentCount;
		nullptr								// pPreserveAttachments;
	};

	VkSubpassDependency dependency{
		VK_SUBPASS_EXTERNAL,														// srcSubpass;
		0,																			// dstSubpass;
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,								// srcStageMask;
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,								// dstStageMask;
		0,																			// srcAccessMask;
		VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,	// dstAccessMask;
		0,																			// dependencyFlags;
	};

	// render pass (contain descriptions)

	VkRenderPassCreateInfo createInfo{
		VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,	// sType;
		nullptr,									// pNext;
		0,											// flags;
		attachments.size(),							// attachmentCount;
		attachments.data(),							// pAttachments;
		1,											// subpassCount;
		&subpass,									// pSubpasses;
		1,											// dependencyCount;
		&dependency,								// pDependencies;
	};

	VkResult result = vkCreateRenderPass(device, &createInfo, nullptr, &renderPass);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create renderpass");
	}
}

void RenderPass::createDepthResources(Device *pDevice, VkExtent2D depthImageExtent, VkFormat depthImagetFormat)
{
	VkExtent3D extent{
		depthImageExtent.width,
		depthImageExtent.height,
		1
	};

	pDepthImage = new Image(
		pDevice,
		extent,
		0,
		1,
		depthImagetFormat,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
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

	pDepthImage->createImageView(subresourceRange, VK_IMAGE_VIEW_TYPE_2D);

	pDepthImage->transitLayout(
		pDevice,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		subresourceRange
	);
}

void RenderPass::createFramebuffers(std::vector<VkImageView> swapChainImageViews)
{
	framebuffers.resize(swapChainImageViews.size());

	VkExtent3D extent = pDepthImage->extent;

	for (int i = 0; i < swapChainImageViews.size(); i++)
	{
		std::vector<VkImageView> attachments{
			swapChainImageViews[i],
			pDepthImage->view
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
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create framebuffers");
		}
	}
}

