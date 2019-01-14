#pragma once
#include "RenderPass.h"
#include "TextureImage.h"

class SsaoRenderPass :
	public RenderPass
{
public:
	SsaoRenderPass(Device *device, VkExtent2D attachmentExtent);

	std::shared_ptr<TextureImage> getSsaoTexture() const;

protected:
    void createAttachments() override;

    void createRenderPass() override;

    void createFramebuffers() override;

private:
	std::shared_ptr<TextureImage> ssaoTexture{};
};

