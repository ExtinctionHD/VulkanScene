#pragma once

#include "RenderPass.h"
#include "TextureImage.h"

class DepthRenderPass : public RenderPass
{
public:
	DepthRenderPass(Device *pDevice, VkExtent2D attachmentExtent);

	TextureImage* getDepthMap() const;

protected:
    void createAttachments() override;

    void createRenderPass() override;

    void createFramebuffers() override;

private:
	TextureImage *pDepthMap{};
    
};
