#pragma once

#include <vulkan/vulkan.h>
#include "File.h"

#include "Device.h"

class GraphicsPipeline
{
public:
	// all stages of graphics pipeline
	VkPipeline pipeline;

	// provides all attachments (color and depth)
	VkRenderPass renderpass;

	VkFormat depthAttachmentFormat;

	GraphicsPipeline(Device *pDevice, VkFormat colorAttachmentFormat, VkDescriptorSetLayout descriptorSetLayout, VkExtent2D viewportExtent);

	~GraphicsPipeline();

private:
	// possible formats of depth attachment
	const std::vector<VkFormat> depthFormats =
	{
		VK_FORMAT_D32_SFLOAT,
		VK_FORMAT_D32_SFLOAT_S8_UINT,
		VK_FORMAT_D24_UNORM_S8_UINT
	};

	// files with shaders code
	const std::string vertShaderPath = File::getExeDir() + "shaders/vert.spv";
	const std::string fragShaderPath = File::getExeDir() + "shaders/frag.spv";

	// device that provide pipeline
	VkDevice device;

	// layout of pipeline resources (descriptors)
	VkPipelineLayout layout;

	void createRenderPass(VkFormat colorAttachmentFormat, VkFormat depthAttachmentFormat);

	void createLayout(VkDescriptorSetLayout descriptorSetLayout);

	void createPipeline(VkExtent2D viewportExtent);
};

