#pragma once

#include "RenderPass.h"
#include "SwapChain.h"
#include "TextureImage.h"

class LightingRenderPass : public RenderPass
{
public:
	LightingRenderPass(Device *device, VkExtent2D attachmentExtent);

	std::shared_ptr<TextureImage> getColorTexture() const;

protected:
    void createAttachments() override;

    void createRenderPass() override;

    void createFramebuffers() override;

private:
	std::shared_ptr<TextureImage> colorTexture;
};

