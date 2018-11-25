#pragma once

#include <vulkan/vulkan.h>
#include "Device.h"
#include "RenderPass.h"
#include "TextureImage.h"

class GeometryRenderPass : public RenderPass
{
public:
	GeometryRenderPass(Device *pDevice, VkExtent2D attachmentExtent);

    uint32_t getColorAttachmentCount() const override;

	std::vector<TextureImage*> getMaps() const;

	Image* getDepthImage() const;

protected:
	void createAttachments() override;

	void createRenderPass() override;

	void createFramebuffers() override;

private:
	TextureImage *pPosMap{};

	TextureImage *pLightSpacePosMap{};

	TextureImage *pNormalMap{};

	TextureImage *pAlbedoMap{};

	Image *pDepthImage{};
};

