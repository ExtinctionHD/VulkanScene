#include "SsaoRenderPass.h"
#include <cassert>

// public:

SsaoRenderPass::SsaoRenderPass(Device *pDevice, VkExtent2D attachmentExtent) :RenderPass(pDevice, attachmentExtent)
{
	sampleCount = pDevice->getSampleCount();
}

// protected:

void SsaoRenderPass::createAttachments()
{
	const VkExtent3D attachmentExtent{
	    extent.width,
	    extent.height,
	    1
	};

	pSsaoMap = new TextureImage(
		pDevice,
		attachmentExtent,
		0,
		sampleCount,
		VK_FORMAT_R8_UNORM,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		VK_IMAGE_ASPECT_COLOR_BIT,
		VK_IMAGE_VIEW_TYPE_2D,
		1
	);
	attachments.push_back(pSsaoMap);
}

void SsaoRenderPass::createRenderPass()
{
	// description of attachments

	VkAttachmentDescription colorAttachmentDesc{
		0,									        // flags;
		pSsaoMap->format,		                // format;
		pSsaoMap->getSampleCount(),			    // samples;
		VK_ATTACHMENT_LOAD_OP_CLEAR,		        // loadOp;
		VK_ATTACHMENT_STORE_OP_STORE,		        // storeOp;
		VK_ATTACHMENT_LOAD_OP_DONT_CARE,	        // stencilLoadOp;
		VK_ATTACHMENT_STORE_OP_DONT_CARE,	        // stencilStoreOp;
		VK_IMAGE_LAYOUT_UNDEFINED,			        // initialLayout;
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,   // finalLayout;
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

void SsaoRenderPass::createFramebuffers()
{
	addFramebuffer({ pSsaoMap->view });
}
