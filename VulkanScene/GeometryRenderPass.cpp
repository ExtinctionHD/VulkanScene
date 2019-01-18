#include <cassert>

#include "GeometryRenderPass.h"

// public:

GeometryRenderPass::GeometryRenderPass(Device *device, VkExtent2D attachmentExtent)
    : RenderPass(device, attachmentExtent, device->getSampleCount())
{
}

std::shared_ptr<TextureImage> GeometryRenderPass::getPosTexture() const
{
	return posTexture;
}

std::shared_ptr<TextureImage> GeometryRenderPass::getNormalTexture() const
{
	return normalTexture;
}

std::shared_ptr<TextureImage> GeometryRenderPass::getAlbedoTexture() const
{
	return albedoTexture;
}

std::shared_ptr<Image> GeometryRenderPass::getDepthImage() const
{
	return depthImage;
}

// protected:

void GeometryRenderPass::createAttachments()
{
	const VkExtent3D attachmentExtent{
		extent.width,
		extent.height,
		1
	};

	posTexture = std::make_shared<TextureImage>(
		device,
		attachmentExtent,
		0,
		sampleCount,
        1,
		VK_FORMAT_R16G16B16A16_SFLOAT,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		1,
		false,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		VK_IMAGE_ASPECT_COLOR_BIT,
		VK_FILTER_LINEAR,
		VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER);

	normalTexture = std::make_shared<TextureImage>(
		device,
		attachmentExtent,
		0,
		sampleCount,
        1,
		VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		1,
		false,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		VK_IMAGE_ASPECT_COLOR_BIT,
		VK_FILTER_LINEAR,
		VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER);

	albedoTexture = std::make_shared<TextureImage>(
		device,
		attachmentExtent,
		0,
        sampleCount,
        1,
        VK_FORMAT_R8G8B8A8_UNORM,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		1,
		false,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		VK_IMAGE_ASPECT_COLOR_BIT,
        VK_FILTER_LINEAR,
		VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER);

	const VkImageSubresourceRange subresourceRange{
		VK_IMAGE_ASPECT_DEPTH_BIT,
		0,
		1,
		0,
		1,
	};
	depthImage = std::make_shared<Image>(
		device,
        attachmentExtent,
        0,
		sampleCount,
        subresourceRange.levelCount,
        depthAttachmentFormat,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        subresourceRange.layerCount,
		false,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		VK_IMAGE_ASPECT_DEPTH_BIT);
	depthImage->transitLayout(
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		subresourceRange);

	attachments = { posTexture, normalTexture, albedoTexture, depthImage };
}

void GeometryRenderPass::createRenderPass()
{
	std::vector<VkAttachmentDescription> attachmentDescriptions;

	for (const auto &image : attachments)
	{
		VkAttachmentDescription attachmentDesc{
		    0,                                 
			image->getFormat(),                           
			image->getSampleCount(),	
		    VK_ATTACHMENT_LOAD_OP_CLEAR,             
		    VK_ATTACHMENT_STORE_OP_STORE,         
		    VK_ATTACHMENT_LOAD_OP_DONT_CARE,         
		    VK_ATTACHMENT_STORE_OP_DONT_CARE,        
		    VK_IMAGE_LAYOUT_UNDEFINED,               
		    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		};

        if (image == depthImage)
        {
			attachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachmentDesc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        }

		attachmentDescriptions.push_back(attachmentDesc);
	}

	std::vector<VkAttachmentReference> colorAttachmentReferences(getColorAttachmentCount());
    for (size_t i = 0; i < colorAttachmentReferences.size(); i++)
    {
        const VkAttachmentReference colorAttachmentRef{
			uint32_t(i),
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		};
		colorAttachmentReferences[i] = colorAttachmentRef;
    }

	VkAttachmentReference depthAttachmentRef{
		uint32_t(attachments.size() - 1),				
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
	};

	VkSubpassDescription subpass{
		0,							
		VK_PIPELINE_BIND_POINT_GRAPHICS,	
		0,									
		nullptr,							
		uint32_t(colorAttachmentReferences.size()),	
		colorAttachmentReferences.data(),	
		nullptr,							
		&depthAttachmentRef,				
		0,									
		nullptr								
	};

    const VkSubpassDependency inputDependency{
		VK_SUBPASS_EXTERNAL,                            
		0,                                              
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,           
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,  
		VK_ACCESS_MEMORY_READ_BIT,                      
		VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | 
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,           
		VK_DEPENDENCY_BY_REGION_BIT,                    
	};

    const VkSubpassDependency outputDependency{
		0,                                    
		VK_SUBPASS_EXTERNAL,                            
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,  
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,           
		VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,           
		VK_ACCESS_MEMORY_READ_BIT,                      
		VK_DEPENDENCY_BY_REGION_BIT,                    
	};

	std::vector<VkSubpassDependency> dependencies{
		inputDependency,
		outputDependency
	};

	VkRenderPassCreateInfo createInfo{
		VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		nullptr,									
		0,											
		uint32_t(attachmentDescriptions.size()),				
		attachmentDescriptions.data(),				
		1,											
		&subpass,									
		uint32_t(dependencies.size()),						
		dependencies.data(),						
	};

    const VkResult result = vkCreateRenderPass(device->get(), &createInfo, nullptr, &renderPass);
	assert(result == VK_SUCCESS);
}

void GeometryRenderPass::createFramebuffers()
{
	std::vector<VkImageView> imageViews;
    for(const auto &image : attachments)
    {
		imageViews.push_back(image->getView());
    }

	addFramebuffer(imageViews);
}
