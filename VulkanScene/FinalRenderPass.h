#pragma once

#include "Image.h"
#include "SwapChain.h"
#include "RenderPass.h"
#include "GeometryRenderPass.h"
#include "LightingRenderPass.h"

class FinalRenderPass : public RenderPass
{
public:
	FinalRenderPass(Device *device, SwapChain *swapChain);

	void saveRenderPasses(GeometryRenderPass *geometryRenderPass, LightingRenderPass *lightingRenderPass);

protected:
    void createAttachments() override;

	void createRenderPass() override;

	void createFramebuffers() override;

private:
    SwapChain *pSwapChain;

	GeometryRenderPass *geometryRenderPass{};

	LightingRenderPass *lightingRenderPass{};

	std::shared_ptr<Image> colorImage{};

	std::shared_ptr<Image> depthImage{};
};

