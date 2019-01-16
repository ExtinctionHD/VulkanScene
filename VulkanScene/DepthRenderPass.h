#pragma once

#include "RenderPass.h"
#include "TextureImage.h"

class DepthRenderPass : public RenderPass
{
public:
	DepthRenderPass(Device *device, VkExtent2D attachmentExtent);

	std::shared_ptr<TextureImage> getDepthTexture() const;

protected:
    void createAttachments() override;

    void createRenderPass() override;

    void createFramebuffers() override;

private:
	std::shared_ptr<TextureImage> depthTexture;
    
};
