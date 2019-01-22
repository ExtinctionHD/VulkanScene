#pragma once

#include <vulkan/vulkan.h>
#include "Device.h"
#include "RenderPass.h"
#include "TextureImage.h"

enum TextureType
{
    POSITION,
    NORMAL,
    ALBEDO
};

class GeometryRenderPass : public RenderPass
{
public:
	GeometryRenderPass(Device *device, VkExtent2D attachmentExtent);

	std::vector<TextureImage*> getGBuffer() const;

	std::shared_ptr<TextureImage> getTexture(TextureType type) const;

	std::shared_ptr<Image> getDepthImage() const;

protected:
	void createAttachments() override;

	void createRenderPass() override;

	void createFramebuffers() override;

private:
	std::vector<std::shared_ptr<TextureImage>> gBuffer;

	std::shared_ptr<Image> depthImage;

	void createGBufferTexture(TextureType type, VkFormat format);
};

