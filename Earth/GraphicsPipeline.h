#pragma once

#include <vulkan/vulkan.h>
#include "File.h"
#include "Image.h"
#include <vector>
#include "SwapChain.h"
#include "RenderPass.h"

#include "Device.h"

class GraphicsPipeline
{
public:
	// all stages of graphics pipeline
	VkPipeline pipeline;

	GraphicsPipeline(VkDevice device, std::vector<VkDescriptorSetLayout> descriptorSetLayouts, RenderPass *pRenderPass);

	~GraphicsPipeline();

	// layout of pipeline resources (descriptors)
	VkPipelineLayout layout;

private:

	// files with shaders code
	const std::string VERT_SHADER_PATH = File::getExeDir() + "shaders/vert.spv";
	const std::string FRAG_SHADER_PATH = File::getExeDir() + "shaders/frag.spv";

	// device that provide pipeline
	VkDevice device;

	void createLayout(std::vector<VkDescriptorSetLayout> descriptorSetLayouts);

	void createPipeline(VkRenderPass renderPass, VkExtent2D viewportExtent);
};

