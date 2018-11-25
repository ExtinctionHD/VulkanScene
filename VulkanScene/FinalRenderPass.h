#pragma once

#include "Image.h"
#include "SwapChain.h"
#include "RenderPass.h"
#include "GeometryRenderPass.h"
#include "LightingRenderPass.h"

class FinalRenderPass : public RenderPass
{
public:
	FinalRenderPass(Device *pDevice, SwapChain *pSwapChain);

    uint32_t getColorAttachmentCount() const override;

	void saveRenderPasses(GeometryRenderPass *pGeometryRenderPass, LightingRenderPass *pLightingRenderPass);

protected:
    void createAttachments() override;

	void createRenderPass() override;

	void createFramebuffers() override;

private:
    SwapChain *pSwapChain;

	GeometryRenderPass *pGeometryRenderPass{};

	LightingRenderPass *pLightingRenderPass{};

	Image *pColorImage{};

	// depth image and its view
	Image *pDepthImage{};
};

