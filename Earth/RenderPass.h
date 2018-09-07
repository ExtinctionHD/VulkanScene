#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include "Image.h"
#include "SwapChain.h"

class RenderPass
{
public:
	RenderPass(Device *pDevice, SwapChain *pSwapChain);
	~RenderPass();

	VkRenderPass renderPass;

	VkExtent2D attachmentsExtent;

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

	// device that provide renderPass
	VkDevice device;

	// depth image and its view
	Image *pDepthImage;

	void createRenderPass(VkFormat colorAttachmentFormat, VkFormat depthAttachmentFormat);

	// create depth image, its view and execute its layout transition
	void createDepthResources(Device *pDevice, VkExtent2D extent, VkFormat depthImagetFormat);

	void createFramebuffers(std::vector<VkImageView> swapChainImageViews);
};

