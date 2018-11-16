#pragma once

#include <vulkan/vulkan.h>
#include "Device.h"
#include "RenderPass.h"
#include "TextureImage.h"

class GeometryRenderPass : public RenderPass
{
public:
	GeometryRenderPass(Device *pDevice, VkExtent2D attachmentExtent);

protected:
	void createAttachments() override;

	void createRenderPass() override;

	void createFramebuffers() override;

private:
	TextureImage *pPosMap{};

	TextureImage *pNormalMap{};

	TextureImage *pColorMap{};

	Image *pDepthImage{};
};

