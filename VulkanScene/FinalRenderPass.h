#pragma once

#include "Image.h"
#include "SwapChain.h"
#include "RenderPass.h"
#include "GeometryRenderPass.h"
#include "LightingRenderPass.h"

class FinalRenderPass : public RenderPass
{
public:
	FinalRenderPass(Device *device, VkExtent2D attachmentExtent);

	void saveRenderPasses(GeometryRenderPass *geometryRenderPass, LightingRenderPass *lightingRenderPass);

protected:
    void createAttachments() override;

	void createRenderPass() override;

	void createFramebuffers() override;

private:
	GeometryRenderPass *geometryRenderPass;

	LightingRenderPass *lightingRenderPass;

	std::shared_ptr<Image> colorImage;

	std::shared_ptr<Image> depthImage;
};

