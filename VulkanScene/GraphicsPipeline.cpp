#include "Vertex.h"

#include "GraphicsPipeline.h"

// public:

GraphicsPipeline::GraphicsPipeline(
	Device *device,
	RenderPass *renderPass,
	const std::vector<VkDescriptorSetLayout> &descriptorSetLayouts,
    const std::vector<VkPushConstantRange> &pushConstantRanges,
	const std::vector<std::shared_ptr<ShaderModule>> &shaderModules,
	const std::vector<VkVertexInputBindingDescription> &bindingDescriptions,
	const std::vector<VkVertexInputAttributeDescription> &attributeDescriptions,
    VkBool32 blendEnable)
{
	this->device = device;
	this->renderPass = renderPass;
	this->shaderModules = shaderModules;
	this->bindingDescriptions = bindingDescriptions;
	this->attributeDescriptions = attributeDescriptions;
	this->blendEnable = blendEnable;

	createLayout(descriptorSetLayouts, pushConstantRanges);

	createPipeline();
}

GraphicsPipeline::~GraphicsPipeline()
{
	vkDestroyPipeline(device->get(), pipeline, nullptr);
	vkDestroyPipelineLayout(device->get(), layout, nullptr);
}

VkPipeline GraphicsPipeline::get() const
{
	return pipeline;
}

VkPipelineLayout GraphicsPipeline::getLayout() const
{
	return layout;
}

void GraphicsPipeline::recreate()
{
	vkDestroyPipeline(device->get(), pipeline, nullptr);
	createPipeline();
}

// private:

void GraphicsPipeline::createLayout(
	std::vector<VkDescriptorSetLayout> descriptorSetLayouts,
	const std::vector<VkPushConstantRange> &pushConstantRanges)
{
	VkPipelineLayoutCreateInfo createInfo{
		VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		nullptr,										
		0,												
		uint32_t(descriptorSetLayouts.size()),					
		descriptorSetLayouts.data(),					
		uint32_t(pushConstantRanges.size()),												
		pushConstantRanges.data(),
	};

    const VkResult result = vkCreatePipelineLayout(device->get(), &createInfo, nullptr, &layout);
	assert(result == VK_SUCCESS);
}

void GraphicsPipeline::createPipeline()
{
	// shader stages info:

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
	for (const auto& shaderModule : shaderModules)
	{
		VkPipelineShaderStageCreateInfo shaderStageCreateInfo{
			VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			nullptr,                                            
			0,                                                  
			shaderModule->getStage(),                           
			shaderModule->getModule(),                          
			"main",                                             
			shaderModule->getSpecializationInfo(),              
		};

		shaderStages.push_back(shaderStageCreateInfo);
	}

    // input stage (load vertices from memory):

	VkPipelineVertexInputStateCreateInfo vertexInputState{
		VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		nullptr,												
		0,														
		uint32_t(bindingDescriptions.size()),								
		bindingDescriptions.data(),								
		uint32_t(attributeDescriptions.size()),					
		attributeDescriptions.data()							
	};

	// assembly stage (converts vertices to primitives):

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState{
		VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		nullptr,													
		0,															
		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,						
		false,													
	};

	// view area:

	VkExtent2D viewportExtent = renderPass->getExtent();

	VkViewport viewport{
		0,
		0,
		float(viewportExtent.width),
		float(viewportExtent.height),
		0,
		1
	};

	VkRect2D scissor{
		{ 0, 0 },
		viewportExtent
	};

	VkPipelineViewportStateCreateInfo viewportState{
		VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		nullptr,												
		0,														
		1,														
		&viewport,												
		1,														
		&scissor												
	};

	// rasterization (converts primitives into fragments):

	VkPipelineRasterizationStateCreateInfo rasterizationState{
		VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		nullptr,					
		0,							
		false,					
		false,					
		VK_POLYGON_MODE_FILL,		
		VK_CULL_MODE_NONE,			
		VK_FRONT_FACE_CLOCKWISE,	
		false,					
		0,							
		0,							
		0,							
		1.0f,		
	};

	// multisampling (create several samples for each fragment):

	VkPipelineMultisampleStateCreateInfo multisampleState{
		VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		nullptr,      
		0,            
		renderPass->getSampleCount(),  
		true,      
		0.2f,         
		nullptr,      
		false,     
		false      
	};

	// depth and stencil tests:

	VkPipelineDepthStencilStateCreateInfo depthStencilState{
		VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
		nullptr,					
		0,							
		true,					
		true,					
		VK_COMPARE_OP_LESS_OR_EQUAL,
		false,					
		false,					
		{},							
		{},							
		0,							
		1							
	};

	// color blending:

	uint32_t colorAttachmentCount = renderPass->getColorAttachmentCount();
	std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments(colorAttachmentCount);
    for (uint32_t i = 0; i < colorAttachmentCount; i++)
    {
		colorBlendAttachments[i] = VkPipelineColorBlendAttachmentState{
			blendEnable,                         
			VK_BLEND_FACTOR_SRC_ALPHA,           
			VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, 
			VK_BLEND_OP_ADD,                     
			VK_BLEND_FACTOR_ONE,                 
			VK_BLEND_FACTOR_ZERO,                
			VK_BLEND_OP_ADD,                     
			VK_COLOR_COMPONENT_R_BIT |
			VK_COLOR_COMPONENT_G_BIT |
			VK_COLOR_COMPONENT_B_BIT |
			VK_COLOR_COMPONENT_A_BIT             
		};
    }

	VkPipelineColorBlendStateCreateInfo colorBlendState{
		VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		nullptr,                
		0,                            
		false,                     
		VK_LOGIC_OP_COPY,             
		uint32_t(colorBlendAttachments.size()), 
		colorBlendAttachments.data(), 
		{ 0, 0, 0, 0 }                
	};

	// pipeline (contains all of the above)

	VkGraphicsPipelineCreateInfo createInfo{
		VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		nullptr,		
		0,					
		uint32_t(shaderStages.size()),
		shaderStages.data(),
		&vertexInputState,	
		&inputAssemblyState,
		nullptr,			
		&viewportState,		
		&rasterizationState,
		&multisampleState,	
		&depthStencilState,	
		&colorBlendState,	
		nullptr,			
		layout,				
		renderPass->get(),  
		0,					
		nullptr,			
		-1,					
	};

	const VkResult result = vkCreateGraphicsPipelines(device->get(), nullptr, 1, &createInfo, nullptr, &pipeline);
	assert(result == VK_SUCCESS);
}

