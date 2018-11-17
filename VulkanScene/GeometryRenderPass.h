#pragma once

#include <vulkan/vulkan.h>
#include "Device.h"
#include "RenderPass.h"
#include "TextureImage.h"

class GeometryRenderPass : public RenderPass
{
public:
	GeometryRenderPass(Device *pDevice, VkExtent2D attachmentExtent);

	std::vector<TextureImage*> getMaps() const;

protected:
	void createAttachments() override;

	void createRenderPass() override;

	void createFramebuffers() override;

private:
	TextureImage *pPosMap{};

	TextureImage *pNormalMap{};

	TextureImage *pAlbedoMap{};

	TextureImage *pSpecularMap{};

	Image *pDepthImage{};
};

