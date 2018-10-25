#pragma once

#include "RenderPass.h"
#include "TextureImage.h"

class ShadowsRenderPass : public RenderPass
{
public:
	ShadowsRenderPass(Device *pDevice, VkExtent2D textureExtent);

	~ShadowsRenderPass();

protected:
    void createRenderPass() override;

    void createFramebuffers() override;

private:
	TextureImage *pShadowTexture{};
    
};
