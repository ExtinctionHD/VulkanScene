#pragma once

#include "RenderPass.h"
#include "TextureImage.h"

class DepthRenderPass : public RenderPass
{
public:
	const uint32_t CASCADE_COUNT = 4;

	DepthRenderPass(Device *device, VkExtent2D attachmentExtent);

	~DepthRenderPass();

	std::shared_ptr<TextureImage> getDepthTexture() const;

    uint32_t getRenderCount() const override;

protected:
    void createAttachments() override;

    void createRenderPass() override;

    void createFramebuffers() override;

private:
	std::shared_ptr<TextureImage> depthTexture;

	std::vector<VkImageView> cascadeViews;

	void destroyCascadeViews();
};
