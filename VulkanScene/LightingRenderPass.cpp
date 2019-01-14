#include <cassert>

#include "LightingRenderPass.h"

// public:

LightingRenderPass::LightingRenderPass(Device *device, SwapChain *swapChain) : RenderPass(device, swapChain->getExtent(), device->getSampleCount())
{
	colorAttachmentFormat = swapChain->getImageFormat();
}

std::shared_ptr<Image> LightingRenderPass::getColorImage() const
{
	return colorImage;
}

// protected:

void LightingRenderPass::createAttachments()
{
	const VkExtent3D attachmentExtent{
		extent.width,
		extent.height,
		1
	};

	const VkImageSubresourceRange subresourceRange{
		VK_IMAGE_ASPECT_COLOR_BIT,
		0,
		1,
		0,
		1,
	};

	colorImage = std::make_shared<Image>(
		device,
		attachmentExtent,
		0,
		sampleCount,
		subresourceRange.levelCount,
		colorAttachmentFormat,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		subresourceRange.layerCount);
	colorImage->createImageView(subresourceRange, VK_IMAGE_VIEW_TYPE_2D);
	colorImage->transitLayout(
		device,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		subresourceRange);

	attachments = { colorImage };
}

void LightingRenderPass::createRenderPass()
{
	// description of attachments

    const VkAttachmentDescription colorAttachmentDesc{
		0,								
		colorImage->format,		                
		colorImage->getSampleCount(),			 
		VK_ATTACHMENT_LOAD_OP_CLEAR,		     
		VK_ATTACHMENT_STORE_OP_STORE,		     
		VK_ATTACHMENT_LOAD_OP_DONT_CARE,	     
		VK_ATTACHMENT_STORE_OP_DONT_CARE,	     
		VK_IMAGE_LAYOUT_UNDEFINED,			     
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
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
		attachmentDescriptions.size(),				
		attachmentDescriptions.data(),				
		1,											
		&subpass,									
		dependencies.size(),						
		dependencies.data(),						
	};

    const VkResult result = vkCreateRenderPass(device->getVk(), &createInfo, nullptr, &renderPass);
	assert(result == VK_SUCCESS);
}

void LightingRenderPass::createFramebuffers()
{
	addFramebuffer({ colorImage->view });
}

