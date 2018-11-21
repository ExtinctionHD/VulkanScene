#pragma once
#include "RenderPass.h"
#include "SwapChain.h"

class LightingRenderPass : public RenderPass
{
public:
	LightingRenderPass(Device *pDevice, SwapChain *pSwapChain);

    uint32_t getColorAttachmentCount() const override;

protected:
    void createAttachments() override;

    void createRenderPass() override;

    void createFramebuffers() override;

private:
	SwapChain *pSwapChain;

	Image *pColorImage{};
};

