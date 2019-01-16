#include "SsaoRenderPass.h"
#include <cassert>

// public:

SsaoRenderPass::SsaoRenderPass(Device *device, VkExtent2D attachmentExtent)
    : RenderPass(device, attachmentExtent, VK_SAMPLE_COUNT_1_BIT)
{
}

std::shared_ptr<TextureImage> SsaoRenderPass::getSsaoTexture() const
{
	return ssaoTexture;
}

// protected:

void SsaoRenderPass::createAttachments()
{
	const VkExtent3D attachmentExtent{
	    extent.width,
	    extent.height,
	    1
	};

	ssaoTexture = std::make_shared<TextureImage>(
		device,
		attachmentExtent,
		0,
		sampleCount,
        1,
		VK_FORMAT_R8_UNORM,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        1,
        false,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		VK_IMAGE_ASPECT_COLOR_BIT,
        VK_FILTER_LINEAR,
		VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER);

	attachments = { ssaoTexture };
}

void SsaoRenderPass::createRenderPass()
{
	// description of attachments

    const VkAttachmentDescription colorAttachmentDesc{
		0,								
		ssaoTexture->getFormat(),		             
		ssaoTexture->getSampleCount(),			 
		VK_ATTACHMENT_LOAD_OP_CLEAR,		     
		VK_ATTACHMENT_STORE_OP_STORE,		     
		VK_ATTACHMENT_LOAD_OP_DONT_CARE,	     
		VK_ATTACHMENT_STORE_OP_DONT_CARE,	     
		VK_IMAGE_LAYOUT_UNDEFINED,			     
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
	};

	std::vector<VkAttachmentDescription> attachmentDescriptions{
		colorAttachmentDesc
	};

	// references to attachments

	VkAttachmentReference colorAttachmentRef{
		0,							
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	};

	// subpass and it dependencies (contain references)

	VkSubpassDescription subpass{
		0,							
		VK_PIPELINE_BIND_POINT_GRAPHICS,	
		0,									
		nullptr,							
		1,									
		&colorAttachmentRef,				
		nullptr,			                
		nullptr,				            
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

	// render pass (contain descriptions)

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

void SsaoRenderPass::createFramebuffers()
{
	addFramebuffer({ ssaoTexture->getView() });
}
