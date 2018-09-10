#pragma once

#include <vulkan/vulkan.h>
#include "File.h"
#include "Image.h"
#include <vector>
#include "SwapChain.h"
#include "RenderPass.h"
#include "ShaderModule.h"

#include "Device.h"

class GraphicsPipeline
{
public:
	// all stages of graphics pipeline
	VkPipeline pipeline;

	GraphicsPipeline(VkDevice device, std::vector<VkDescriptorSetLayout> descriptorSetLayouts, RenderPass *pRenderPass, std::vector<ShaderModule*> shaderModules);

	~GraphicsPipeline();

	// layout of pipeline resources (descriptors)
	VkPipelineLayout layout;

	// recreate with new render pass
	void recreate(RenderPass *pRenderPass);

private:

	// device that provide pipeline
	VkDevice device;

	std::vector<ShaderModule*> shaderModules;

	void createLayout(std::vector<VkDescriptorSetLayout> descriptorSetLayouts);

	void createPipeline(VkRenderPass renderPass, VkExtent2D viewportExtent);
};

