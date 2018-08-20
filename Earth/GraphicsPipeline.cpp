#include <array>
#include "Logger.h"

#include "GraphicsPipeline.h"

// public:

GraphicsPipeline::GraphicsPipeline(Device *pDevice, VkFormat colorAttachmentFormat)
{
	device = pDevice->device;

	VkFormat depthAttachmentFormat = pDevice->findSupportedFormat(
		depthFormats, 
		VK_IMAGE_TILING_OPTIMAL, 
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);
	createRenderPass(colorAttachmentFormat, depthAttachmentFormat);
}

GraphicsPipeline::~GraphicsPipeline()
{
	vkDestroyRenderPass(device, renderpass, nullptr);
}

// private:

void GraphicsPipeline::createRenderPass(VkFormat colorAttachmentFormat, VkFormat depthAttachmentFormat)
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

	std::array<VkAttachmentDescription, 2> attachments{
		colorAttachment,
		depthAttachment
	};

	// references to attachments

	VkAttachmentReference colorAttachmentRef{
		0,											// attachment;
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL	// layout;
	};

	VkAttachmentReference depthAttachmentRef{
		0,													// attachment;
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

	VkResult result = vkCreateRenderPass(device, &createInfo, nullptr, &renderpass);
	if (result != VK_SUCCESS)
	{
		LOGGER_FATAL(Logger::FAILED_TO_CREATE_RENDER_PASS);
	}
}
