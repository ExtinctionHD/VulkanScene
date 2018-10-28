#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include "RenderPass.h"
#include "ShaderModule.h"

class GraphicsPipeline
{
public:
	// all stages of graphics pipeline
	VkPipeline pipeline{};

	GraphicsPipeline(
		VkDevice device, 
		std::vector<VkDescriptorSetLayout> descriptorSetLayouts, 
		RenderPass *pRenderPass, 
		std::vector<ShaderModule*> shaderModules,
		VkVertexInputBindingDescription bindingDescription,
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions
	);

	~GraphicsPipeline();

	// layout of pipeline resources (descriptors)
	VkPipelineLayout layout{};

	// recreate with new render pass
	void recreate();

private:
	// device that provide pipeline
	VkDevice device;

	RenderPass *pRenderPass;

	// shader modules for this pipeline
	std::vector<ShaderModule*> shaderModules;

	// information about input vertices
	VkVertexInputBindingDescription bindingDescription{};
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

	void createLayout(std::vector<VkDescriptorSetLayout> descriptorSetLayouts);

	void createPipeline(VkExtent2D viewportExtent);
};

