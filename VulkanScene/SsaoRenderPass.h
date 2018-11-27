#pragma once
#include "RenderPass.h"
#include "TextureImage.h"

class SsaoRenderPass :
	public RenderPass
{
public:
	SsaoRenderPass(Device *pDevice, VkExtent2D attachmentExtent);

protected:
    void createAttachments() override;

    void createRenderPass() override;

    void createFramebuffers() override;

private:
	TextureImage *pSsaoMap{};
};

