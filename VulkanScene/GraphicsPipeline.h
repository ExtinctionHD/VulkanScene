#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include "RenderPass.h"
#include "ShaderModule.h"
#include <memory>

class GraphicsPipeline
{
public:
	// all stages of graphics pipeline
	VkPipeline pipeline{};

	GraphicsPipeline(
		Device *pDevice, 
		std::vector<VkDescriptorSetLayout> descriptorSetLayouts, 
		RenderPass *pRenderPass, 
		std::vector<std::shared_ptr<ShaderModule>> shaderModules,
		std::vector<VkVertexInputBindingDescription> bindingDescriptions,
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions,
		VkSampleCountFlagBits sampleCount,
        uint32_t colorAttachmentCount,
        VkBool32 blendEnable);

	~GraphicsPipeline();

	// layout of pipeline resources (descriptors)
	VkPipelineLayout layout{};

	// recreate with new render pass
	void recreate();

private:
	// device that provide pipeline
	Device *pDevice;

	RenderPass *pRenderPass;

	// shader modules for this pipeline
	std::vector<std::shared_ptr<ShaderModule>> shaderModules;

	// information about input vertices
	std::vector<VkVertexInputBindingDescription> bindingDescriptions;
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

	VkSampleCountFlagBits sampleCount;

	uint32_t attachmentCount;

	VkBool32 blendEnable;

	void createLayout(std::vector<VkDescriptorSetLayout> descriptorSetLayouts);

	void createPipeline(VkExtent2D viewportExtent);
};

