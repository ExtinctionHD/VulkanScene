#pragma once

#include <vulkan/vulkan.h>

#include "Device.h"

class GraphicsPipeline
{
public:
	VkPipeline pipeline;

	VkFormat depthAttachmentFormat;

	GraphicsPipeline(Device *pDevice, VkFormat colorAttachmentFormat, VkDescriptorSetLayout descriptorSetLayout);

	~GraphicsPipeline();

private:
	// possible formats of depth attachment
	const std::vector<VkFormat> depthFormats =
	{
		VK_FORMAT_D32_SFLOAT,
		VK_FORMAT_D32_SFLOAT_S8_UINT,
		VK_FORMAT_D24_UNORM_S8_UINT
	};

	// device that provide pipeline
	VkDevice device;

	// layout of pipeline resources (descriptors)
	VkPipelineLayout layout;
	
	// provides all attachments (color and depth)
	VkRenderPass renderpass;

	void createRenderPass(VkFormat colorAttachmentFormat, VkFormat depthAttachmentFormat);

	void createLayout(VkDescriptorSetLayout descriptorSetLayout);
};

