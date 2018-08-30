#pragma once

#include <vulkan/vulkan.h>
#include "File.h"
#include "Image.h"
#include <vector>
#include "SwapChain.h"

#include "Device.h"

class GraphicsPipeline
{
public:
	// all stages of graphics pipeline
	VkPipeline pipeline;

	GraphicsPipeline(Device *pDevice, SwapChain *pSwapChain, VkDescriptorSetLayout descriptorSetLayout);

	~GraphicsPipeline();

	// provides all attachments (color and depth)
	VkRenderPass renderpass;

	// layout of pipeline resources (descriptors)
	VkPipelineLayout layout;

	// destination images for rendering, 
	// framebuffer attachments: image view and depth image
	std::vector<VkFramebuffer> framebuffers;

private:
	// possible formats of depth attachment
	const std::vector<VkFormat> DEPTH_FORMATS =
	{
		VK_FORMAT_D32_SFLOAT,
		VK_FORMAT_D32_SFLOAT_S8_UINT,
		VK_FORMAT_D24_UNORM_S8_UINT
	};

	// files with shaders code
	const std::string VERT_SHADER_PATH = File::getExeDir() + "shaders/vert.spv";
	const std::string FRAG_SHADER_PATH = File::getExeDir() + "shaders/frag.spv";

	// device that provide pipeline
	VkDevice device;

	// depth image and its view
	Image *pDepthImage;

	void createRenderPass(VkFormat colorAttachmentFormat, VkFormat depthAttachmentFormat);

	// create depth image, its view and execute its layout transition
	void createDepthResources(Device *pDevice, VkExtent2D extent, VkFormat depthImagetFormat);

	void createLayout(VkDescriptorSetLayout descriptorSetLayout);

	void createPipeline(VkExtent2D viewportExtent);

	void createFramebuffers(std::vector<VkImageView> swapChainImageViews);
};

