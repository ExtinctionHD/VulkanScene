#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include "Image.h"
#include "SwapChain.h"
#include "RenderPass.h"

class FinalRenderPass : public RenderPass
{
public:
	FinalRenderPass(Device *pDevice, SwapChain *pSwapChain);
	~FinalRenderPass();

protected:
	void createRenderPass() override;

	void createFramebuffers() override;

private:
    SwapChain *pSwapChain;

	VkFormat depthAttachmentFormat;

	// depth image and its view
	Image *pDepthImage;

	// create depth image, its view and execute its layout transition
	void createDepthResources();

};

