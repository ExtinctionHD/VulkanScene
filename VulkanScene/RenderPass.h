#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include "Device.h"

class RenderPass
{
public:
    virtual ~RenderPass();

	VkRenderPass getRenderPass() const;

	std::vector<VkFramebuffer> getFramebuffers() const;

	VkExtent2D getExtent() const;

	void create();

	void recreate(VkExtent2D newExtent);

protected:
	RenderPass(Device *pDevice, VkExtent2D extent);

	// possible formats of depth attachment
	const std::vector<VkFormat> DEPTH_FORMATS{
		VK_FORMAT_D32_SFLOAT,
		VK_FORMAT_D32_SFLOAT_S8_UINT,
		VK_FORMAT_D24_UNORM_S8_UINT
	};

	Device *pDevice;

	VkRenderPass renderPass{};

	// destination images for rendering
	std::vector<VkFramebuffer> framebuffers;

	VkExtent2D extent{};

	VkFormat depthAttachmentFormat;

	virtual void createRenderPass() = 0;

	virtual void createFramebuffers() = 0;

private:
	void cleanup();
};
