#pragma once

#include <vulkan/vulkan.h>
#include "Device.h"
#include "RenderPass.h"
#include "TextureImage.h"

class GeometryRenderPass : public RenderPass
{
public:
	GeometryRenderPass(Device *device, VkExtent2D attachmentExtent);

	std::shared_ptr<TextureImage> getPosTexture() const;

	std::shared_ptr<TextureImage> getNormalTexture() const;

	std::shared_ptr<TextureImage> getAlbedoTexture() const;

	std::shared_ptr<Image> getDepthImage() const;

protected:
	void createAttachments() override;

	void createRenderPass() override;

	void createFramebuffers() override;

private:
	std::shared_ptr<TextureImage> posTexture{};

	std::shared_ptr<TextureImage> normalTexture{};

	std::shared_ptr<TextureImage> albedoTexture{};

	std::shared_ptr<Image> depthImage{};
};

