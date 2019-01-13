#include "GeometryRenderPass.h"
#include <cassert>

// public:

GeometryRenderPass::GeometryRenderPass(Device *pDevice, VkExtent2D attachmentExtent) : RenderPass(pDevice, attachmentExtent)
{
	sampleCount = pDevice->getSampleCount();
}

uint32_t GeometryRenderPass::getColorAttachmentCount() const
{
	return attachments.size() - 1;
}

TextureImage * GeometryRenderPass::getPosMap() const
{
	return pPosMap;
}

TextureImage * GeometryRenderPass::getNormalMap() const
{
	return pNormalMap;
}

TextureImage * GeometryRenderPass::getAlbedoMap() const
{
	return pAlbedoMap;
}

Image * GeometryRenderPass::getDepthImage() const
{
	return pDepthImage;
}

// protected:

void GeometryRenderPass::createAttachments()
{
	const VkExtent3D attachmentExtent{
		extent.width,
		extent.height,
		1
	};

	pPosMap = new TextureImage(
		pDevice,
		attachmentExtent,
		0,
		sampleCount,
		VK_FORMAT_R16G16B16A16_SFLOAT,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		VK_IMAGE_ASPECT_COLOR_BIT,
		VK_IMAGE_VIEW_TYPE_2D,
		1,
        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER
	);
	attachments.push_back(pPosMap);

	pNormalMap = new TextureImage(
		pDevice,
		attachmentExtent,
		0,
		sampleCount,
		VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		VK_IMAGE_ASPECT_COLOR_BIT,
		VK_IMAGE_VIEW_TYPE_2D,
		1,
		VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER
	);
	attachments.push_back(pNormalMap);

	pAlbedoMap = new TextureImage(
		pDevice,
		attachmentExtent,
		0,
		sampleCount,
		VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		VK_IMAGE_ASPECT_COLOR_BIT,
		VK_IMAGE_VIEW_TYPE_2D,
		1,
		VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER
	);
	attachments.push_back(pAlbedoMap);

	pDepthImage = new Image(
		pDevice,
        attachmentExtent,
        0,
		sampleCount,
        1,
        depthAttachmentFormat,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        1
	);
	attachments.push_back(pDepthImage);

	VkImageSubresourceRange subresourceRange = VkImageSubresourceRange{
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

void GeometryRenderPass::createRenderPass()
{
	std::vector<VkAttachmentDescription> attachmentDescriptions;

	// Init attachment properties
	for (Image *pImage : attachments)
	{
		VkAttachmentDescription attachmentDesc{
		    0,                                          // flags;
		    pImage->format,                             // format;
		    pImage->getSampleCount(),					// samples;
		    VK_ATTACHMENT_LOAD_OP_CLEAR,                // loadOp;
		    VK_ATTACHMENT_STORE_OP_STORE,               // storeOp;
		    VK_ATTACHMENT_LOAD_OP_DONT_CARE,            // stencilLoadOp;
		    VK_ATTACHMENT_STORE_OP_DONT_CARE,           // stencilStoreOp;
		    VK_IMAGE_LAYOUT_UNDEFINED,                  // initialLayout;
		    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,   // finalLayout;
		};

        if (pImage == pDepthImage)
        {
			attachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachmentDesc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        }

		attachmentDescriptions.push_back(attachmentDesc);
	}

	std::vector<VkAttachmentReference> colorAttachmentReferences;

    for (size_t i = 0; i < attachments.size() - 1; i++)
    {
		VkAttachmentReference colorAttachmentRef{
			i,                                          // attachment;
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL    // layout;
		};
		colorAttachmentReferences.push_back(colorAttachmentRef);
    }

	VkAttachmentReference depthAttachmentRef{
		attachments.size() - 1,								// attachment;
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL	// layout;
	};

	VkSubpassDescription subpass{
		0,									// flags;
		VK_PIPELINE_BIND_POINT_GRAPHICS,	// pipelineBindPoint;
		0,									// inputAttachmentCount;
		nullptr,							// pInputAttachmentReferences;
		colorAttachmentReferences.size(),	// colorAttachmentCount;
		colorAttachmentReferences.data(),	// pColorAttachmentReferences;
		nullptr,							// pResolveAttachmentReference;
		&depthAttachmentRef,				// pDepthStencilAttachmentReference;
		0,									// preserveAttachmentCount;
		nullptr								// pPreserveAttachments;
	};

	VkSubpassDependency inputDependency{
		VK_SUBPASS_EXTERNAL,                            // srcSubpass;
		0,                                              // dstSubpass;
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,           // srcStageMask;
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,  // dstStageMask;
		VK_ACCESS_MEMORY_READ_BIT,                      // srcAccessMask;
		VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | 
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,           // dstAccessMask;
		VK_DEPENDENCY_BY_REGION_BIT,                    // dependencyFlags;
	};

	VkSubpassDependency outputDependency{
		0,                                              // srcSubpass;
		VK_SUBPASS_EXTERNAL,                            // dstSubpass;
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,  // srcStageMask;
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,           // dstStageMask;
		VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,           // srcAccessMask;
		VK_ACCESS_MEMORY_READ_BIT,                      // dstAccessMask;
		VK_DEPENDENCY_BY_REGION_BIT,                    // dependencyFlags;
	};

	std::vector<VkSubpassDependency> dependencies{
		inputDependency,
		outputDependency
	};

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

	VkResult result = vkCreateRenderPass(pDevice->getVk(), &createInfo, nullptr, &renderPass);
	assert(result == VK_SUCCESS);
}

void GeometryRenderPass::createFramebuffers()
{
	std::vector<VkImageView> imageViews;
    for(Image *pImage : attachments)
    {
		imageViews.push_back(pImage->view);
    }

	addFramebuffer(imageViews);
}
