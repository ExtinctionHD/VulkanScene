#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include "RenderPass.h"
#include "ShaderModule.h"
#include <memory>

class GraphicsPipeline
{
public:
	GraphicsPipeline(
		Device *device,
		RenderPass *renderPass,
		const std::vector<VkDescriptorSetLayout> &descriptorSetLayouts,
		const std::vector<std::shared_ptr<ShaderModule>> &shaderModules,
		const std::vector<VkVertexInputBindingDescription> &bindingDescriptions,
		const std::vector<VkVertexInputAttributeDescription> &attributeDescriptions,
		VkBool32 blendEnable);

	~GraphicsPipeline();

	VkPipeline get() const;

	VkPipelineLayout getLayout() const;

	void recreate();

private:
	Device *device;

	RenderPass *renderPass;

	VkPipeline pipeline;

	VkPipelineLayout layout;

	std::vector<std::shared_ptr<ShaderModule>> shaderModules;

	std::vector<VkVertexInputBindingDescription> bindingDescriptions;
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

	VkBool32 blendEnable;

	void createLayout(std::vector<VkDescriptorSetLayout> descriptorSetLayouts);

	void createPipeline();
};

