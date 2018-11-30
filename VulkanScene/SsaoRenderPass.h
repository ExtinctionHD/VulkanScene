#pragma once
#include "RenderPass.h"
#include "TextureImage.h"

class SsaoRenderPass :
	public RenderPass
{
public:
	SsaoRenderPass(Device *pDevice, VkExtent2D attachmentExtent);

	TextureImage* getSsaoMap() const;

protected:
    void createAttachments() override;

    void createRenderPass() override;

    void createFramebuffers() override;

private:
	TextureImage *pSsaoMap{};
};

