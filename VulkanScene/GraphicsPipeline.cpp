#include "Vertex.h"

#include "GraphicsPipeline.h"

// public:

GraphicsPipeline::GraphicsPipeline(
	Device *pDevice,
	std::vector<VkDescriptorSetLayout> descriptorSetLayouts, 
	RenderPass *pRenderPass, 
	std::vector<std::shared_ptr<ShaderModule>> shaderModules,
	std::vector<VkVertexInputBindingDescription> bindingDescriptions,
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions,
    VkSampleCountFlagBits sampleCount,
    uint32_t colorAttachmentCount,
    VkBool32 blendEnable)
{
	this->pDevice = pDevice;
	this->pRenderPass = pRenderPass;
	this->shaderModules = shaderModules;
	this->bindingDescriptions = bindingDescriptions;
	this->attributeDescriptions = attributeDescriptions;
	this->sampleCount = sampleCount;
	this->attachmentCount = colorAttachmentCount;
	this->blendEnable = blendEnable;

	createLayout(descriptorSetLayouts);

	createPipeline(pRenderPass->getExtent());
}

GraphicsPipeline::~GraphicsPipeline()
{
	vkDestroyPipeline(pDevice->getVk(), pipeline, nullptr);
	vkDestroyPipelineLayout(pDevice->getVk(), layout, nullptr);
}

void GraphicsPipeline::recreate()
{
	vkDestroyPipeline(pDevice->getVk(), pipeline, nullptr);
	createPipeline(pRenderPass->getExtent());
}

// private:

void GraphicsPipeline::createLayout(std::vector<VkDescriptorSetLayout> descriptorSetLayouts)
{
	VkPipelineLayoutCreateInfo createInfo{
		VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,	// sType;
		nullptr,										// pNext;
		0,												// flags;
		descriptorSetLayouts.size(),					// setLayoutCount;
		descriptorSetLayouts.data(),					// pSetLayouts;
		0,												// pushConstantRangeCount;
		nullptr,										// pPushConstantRanges;
	};

	VkResult result = vkCreatePipelineLayout(pDevice->getVk(), &createInfo, nullptr, &layout);
	assert(result == VK_SUCCESS);
}

void GraphicsPipeline::createPipeline(VkExtent2D viewportExtent)
{
	// shader stages info:

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
	for (const auto& shaderModule : shaderModules)
	{
		VkPipelineShaderStageCreateInfo shaderStageCreateInfo{
			VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,    // sType;
			nullptr,                                                // pNext;
			0,                                                      // flags;
			shaderModule->getStage(),                              // stage;
			shaderModule->getModule(),                             // module;
			"main",                                                 // pName;
			shaderModule->getSpecializationInfo(),                 // pSpecializationInfo;
		};

		shaderStages.push_back(shaderStageCreateInfo);
	}

	VkPipelineVertexInputStateCreateInfo vertexInputState{
		VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,	// sType;
		nullptr,													// pNext;
		0,															// flags;
		bindingDescriptions.size(),									// vertexBindingDescriptionCount;
		bindingDescriptions.data(),									// pVertexBindingDescriptions;
		uint32_t(attributeDescriptions.size()),						// vertexAttributeDescriptionCount;
		attributeDescriptions.data()								// pVertexAttributeDescriptions;
	};

	// assembly stage (converts vertices to primitives):

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState{
		VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,	// sType;
		nullptr,														// pNext;
		0,																// flags;
		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,							// topology;
		VK_FALSE,														// primitiveRestartEnable;
	};

	// view area:

	VkViewport viewport{
		0,
		0,
		float(viewportExtent.width),
		float(viewportExtent.height),
		0,
		1
	};

	VkRect2D scissor{
		{ 0, 0 },		// offset
		viewportExtent	// extent
	};

	VkPipelineViewportStateCreateInfo viewportState{
		VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,	// sType;
		nullptr,												// pNext;
		0,														// flags;
		1,														// viewportCount;
		&viewport,												// pViewports;
		1,														// scissorCount;
		&scissor												// pScissors;
	};

	// rasterization (converts primitives into fragments):

	VkPipelineRasterizationStateCreateInfo rasterizationState{
		VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,	// sType;
		nullptr,					// pNext;
		0,							// flags;
		VK_FALSE,					// depthClampEnable;
		VK_FALSE,					// rasterizerDiscardEnable;
		VK_POLYGON_MODE_FILL,		// polygonMode;
		VK_CULL_MODE_NONE,			// cullMode;
		VK_FRONT_FACE_CLOCKWISE,	// frontFace;
		VK_FALSE,					// depthBiasEnable;
		0,							// depthBiasConstantFactor;
		0,							// depthBiasClamp;
		0,							// depthBiasSlopeFactor;
		1.0f,						// lineWidth;
	};

	// multisampling (create several samples for each fragment):

	VkPipelineMultisampleStateCreateInfo multisampleState{
		VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,	// sType;
		nullptr,        // pNext;
		0,              // flags;
		sampleCount,    // rasterizationSamples;
		VK_TRUE,        // sampleShadingEnable;
		0.2f,           // minSampleShading;
		nullptr,        // pSampleMask;
		VK_FALSE,       // alphaToCoverageEnable;
		VK_FALSE        // alphaToOneEnable;
	};

	// depth and stencil tests:

	VkPipelineDepthStencilStateCreateInfo depthStencilState{
		VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,	// sType;
		nullptr,						// pNext;
		0,								// flags;
		VK_TRUE,						// depthTestEnable;
		VK_TRUE,						// depthWriteEnable;
		VK_COMPARE_OP_LESS_OR_EQUAL,	// depthCompareOp;
		VK_FALSE,						// depthBoundsTestEnable;
		VK_FALSE,						// stencilTestEnable;
		{},								// front;
		{},								// back;
		0,								// minDepthBounds;
		1								// maxDepthBounds;
	};

	// color blending:

	std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments(attachmentCount);
    for (uint32_t i = 0; i < attachmentCount; i++)
    {
		colorBlendAttachments[i] = VkPipelineColorBlendAttachmentState{
			blendEnable,                            // blendEnable;
			VK_BLEND_FACTOR_SRC_ALPHA,              // srcColorBlendFactor;
			VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,    // dstColorBlendFactor;
			VK_BLEND_OP_ADD,                        // colorBlendOp;
			VK_BLEND_FACTOR_ONE,                    // srcAlphaBlendFactor;
			VK_BLEND_FACTOR_ZERO,                   // dstAlphaBlendFactor;
			VK_BLEND_OP_ADD,                        // alphaBlendOp;
			VK_COLOR_COMPONENT_R_BIT |
			VK_COLOR_COMPONENT_G_BIT |
			VK_COLOR_COMPONENT_B_BIT |
			VK_COLOR_COMPONENT_A_BIT                // colorWriteMask;
		};
    }

	VkPipelineColorBlendStateCreateInfo colorBlendState{
		VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,   // sType;
		nullptr,                        // pNext;
		0,                              // flags;
		VK_FALSE,                       // logicOpEnable;
		VK_LOGIC_OP_COPY,               // logicOp;
		colorBlendAttachments.size(),   // attachmentCount;
		colorBlendAttachments.data(),   // pAttachments;
		{ 0, 0, 0, 0 }                  // blendConstants[4];
	};

	// pipeline (contains all of the above)

	VkGraphicsPipelineCreateInfo createInfo{
		VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,	// sType;
		nullptr,					    // pNext;
		0,							    // flags;
		shaderStages.size(),		    // stageCount;
		shaderStages.data(),		    // pStages;
		&vertexInputState,			    // pVertexInputState;
		&inputAssemblyState,		    // pInputAssemblyState;
		nullptr,					    // pTessellationState;
		&viewportState,				    // pViewportState;
		&rasterizationState,		    // pRasterizationState;
		&multisampleState,			    // pMultisampleState;
		&depthStencilState,			    // pDepthStencilState;
		&colorBlendState,			    // pColorBlendState;
		nullptr,					    // pDynamicState;
		layout,						    // layout;
		pRenderPass->getVk(),   // renderPass;
		0,							    // subpass;
		nullptr,				    // basePipelineHandle;
		-1,							    // basePipelineIndex;
	};

	VkResult result = vkCreateGraphicsPipelines(pDevice->getVk(), nullptr, 1, &createInfo, nullptr, &pipeline);
	assert(result == VK_SUCCESS);
}

