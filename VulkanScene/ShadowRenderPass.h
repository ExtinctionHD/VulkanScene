#pragma once

#include "RenderPass.h"

class ShadowRenderPass : public RenderPass
{
public:
	ShadowRenderPass(Device *pDevice, VkExtent2D textureExtent);

	~ShadowRenderPass();

protected:
    void createRenderPass() override;

    void createFramebuffers() override;

private:
    
};
