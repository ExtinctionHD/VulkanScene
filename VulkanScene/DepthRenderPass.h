#pragma once

#include "RenderPass.h"
#include "TextureImage.h"

class DepthRenderPass : public RenderPass
{
public:
	DepthRenderPass(Device *pDevice, VkExtent2D textureExtent);

	~DepthRenderPass();

	TextureImage* getDepthMap() const;

protected:
    void createRenderPass() override;

    void createFramebuffers() override;

private:
	TextureImage *pDepthMap{};
    
};
