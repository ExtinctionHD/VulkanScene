#pragma once

#include "Image.h"
#include "SwapChain.h"
#include "RenderPass.h"

class FinalRenderPass : public RenderPass
{
public:
	FinalRenderPass(Device *pDevice, SwapChain *pSwapChain);

protected:
    void createAttachments() override;

	void createRenderPass() override;

	void createFramebuffers() override;

private:
    SwapChain *pSwapChain;

	Image *pColorImage{};

	// depth image and its view
	Image *pDepthImage{};
};

