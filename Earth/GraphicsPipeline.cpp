#include "Logger.h"
#include "ShaderModule.h"
#include "Vertex.h"

#include "GraphicsPipeline.h"

// public:

GraphicsPipeline::GraphicsPipeline(Device *pDevice, VkFormat colorAttachmentFormat, VkDescriptorSetLayout descriptorSetLayout, VkExtent2D viewportExtent)
{
	device = pDevice->device;

	depthAttachmentFormat = pDevice->findSupportedFormat(
		depthFormats, 
		VK_IMAGE_TILING_OPTIMAL, 
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);

	createRenderPass(colorAttachmentFormat, depthAttachmentFormat);

	createLayout(descriptorSetLayout);

	createPipeline(viewportExtent);
}

GraphicsPipeline::~GraphicsPipeline()
{
	vkDestroyPipeline(device, pipeline, nullptr);
	vkDestroyPipelineLayout(device, layout, nullptr);
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

void GraphicsPipeline::createLayout(VkDescriptorSetLayout descriptorSetLayout)
{
	std::vector<VkDescriptorSetLayout> setLayouts = { descriptorSetLayout };

	VkPipelineLayoutCreateInfo createInfo{
		VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,	// sType;
		nullptr,										// pNext;
		0,												// flags;
		setLayouts.size(),								// setLayoutCount;
		setLayouts.data(),								// pSetLayouts;
		0,												// pushConstantRangeCount;
		nullptr,										// pPushConstantRanges;
	};

	VkResult result = vkCreatePipelineLayout(device, &createInfo, nullptr, &layout);
	if (result != VK_SUCCESS)
	{
		LOGGER_FATAL(Logger::FAILED_TO_CREATE_PIPELINE_LAYOUT);
	}
}

void GraphicsPipeline::createPipeline(VkExtent2D viewportExtent)
{
	// loading shaders (vertex and fragment stage):

	ShaderModule vertShaderModule{ device, vertShaderPath };
	VkPipelineShaderStageCreateInfo vertShaderStageCreateInfo{
		VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,	// sType;
		nullptr,												// pNext;
		0,														// flags;
		VK_SHADER_STAGE_VERTEX_BIT,								// stage;
		vertShaderModule,										// module;
		"main",													// pName;
		nullptr,												// pSpecializationInfo;
	};

	ShaderModule fragShaderModule{ device, fragShaderPath };
	VkPipelineShaderStageCreateInfo fragShaderStageCreateInfo{
		VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,	// sType;
		nullptr,												// pNext;
		0,														// flags;
		VK_SHADER_STAGE_FRAGMENT_BIT,							// stage;
		fragShaderModule,										// module;
		"main",													// pName;
		nullptr,												// pSpecializationInfo;
	};

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages = { vertShaderStageCreateInfo, fragShaderStageCreateInfo };

	// info about input vertices:

	uint32_t binding = 0;

	VkVertexInputBindingDescription bindingDescription = Vertex::getBindingDescription(binding);
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions = Vertex::getAttributeDescriptions(binding);

	VkPipelineVertexInputStateCreateInfo vertexInputState{
		VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,	// sType;
		nullptr,													// pNext;
		0,															// flags;
		1,															// vertexBindingDescriptionCount;
		&bindingDescription,										// pVertexBindingDescriptions;
		(uint32_t)attributeDescriptions.size(),								// vertexAttributeDescriptionCount;
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
		0,				// x;
		0,				// y;
		viewportExtent.width,	// width;
		viewportExtent.height,	// height;
		0,				// minDepth;
		1				// maxDepth;
	};

	VkRect2D scissor{
		{ 0, 0 },	// offset
		viewportExtent		// extent
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
		0,							// lineWidth;
	};

	// multisampling (create several samples for each fragment):

	VkPipelineMultisampleStateCreateInfo multisampleState{
		VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,	// sType;
		nullptr,				// pNext;
		0,						// flags;
		VK_SAMPLE_COUNT_1_BIT,	// rasterizationSamples;
		VK_FALSE,				// sampleShadingEnable;
		1,						// minSampleShading;
		nullptr,				// pSampleMask;
		VK_FALSE,				// alphaToCoverageEnable;
		VK_FALSE				// alphaToOneEnable;
	};

	// depth and stencil tests:

	VkPipelineDepthStencilStateCreateInfo depthStencilState{
		VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,	// sType;
		nullptr,			// pNext;
		0,					// flags;
		VK_TRUE,			// depthTestEnable;
		VK_TRUE,			// depthWriteEnable;
		VK_COMPARE_OP_LESS,	// depthCompareOp;
		VK_FALSE,			// depthBoundsTestEnable;
		VK_FALSE,			// stencilTestEnable;
		{},					// front;
		{},					// back;
		0,					// minDepthBounds;
		1					// maxDepthBounds;
	};

	// color blending:

	VkPipelineColorBlendAttachmentState colorBlendAttachment{
		VK_TRUE,								// blendEnable;
		VK_BLEND_FACTOR_SRC_ALPHA,				// srcColorBlendFactor;
		VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,	// dstColorBlendFactor;
		VK_BLEND_OP_ADD,						// colorBlendOp;
		VK_BLEND_FACTOR_ONE,					// srcAlphaBlendFactor;
		VK_BLEND_FACTOR_ZERO,					// dstAlphaBlendFactor;
		VK_BLEND_OP_ADD,						// alphaBlendOp;
		VK_COLOR_COMPONENT_R_BIT |
		VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT |
		VK_COLOR_COMPONENT_A_BIT				// colorWriteMask;
	};

	VkPipelineColorBlendStateCreateInfo colorBlendState{
		VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,	// sType;
		nullptr,				// pNext;
		0,						// flags;
		VK_FALSE,				// logicOpEnable;
		VK_LOGIC_OP_COPY,		// logicOp;
		1,						// attachmentCount;
		&colorBlendAttachment,	// pAttachments;
		{ 0, 0, 0, 0 }			// blendConstants[4];
	};

	// pipeline (contains all of the above)

	VkGraphicsPipelineCreateInfo createInfo{
		VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,	// sType;
		nullptr,					// pNext;
		0,							// flags;
		shaderStages.size(),		// stageCount;
		shaderStages.data(),		// pStages;
		&vertexInputState,			// pVertexInputState;
		&inputAssemblyState,		// pInputAssemblyState;
		nullptr,					// pTessellationState;
		&viewportState,				// pViewportState;
		&rasterizationState,		// pRasterizationState;
		&multisampleState,			// pMultisampleState;
		&depthStencilState,			// pDepthStencilState;
		&colorBlendState,			// pColorBlendState;
		nullptr,					// pDynamicState;
		layout,						// layout;
		renderpass,					// renderPass;
		0,							// subpass;
		VK_NULL_HANDLE,				// basePipelineHandle;
		-1,							// basePipelineIndex;
	};

	VkResult result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &createInfo, nullptr, &pipeline);
	if (result != VK_SUCCESS)
	{
		LOGGER_FATAL(Logger::FAILED_TO_CREATE_GRAPHICS_PIPELINE);
	}
}


