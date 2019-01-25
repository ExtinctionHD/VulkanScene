#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include "Device.h"
#include <unordered_map>
#include "Image.h"
#include <memory>

class RenderPass
{
public:
    virtual ~RenderPass();

	VkRenderPass get() const;

	std::vector<VkFramebuffer> getFramebuffers() const;

	VkExtent2D getExtent() const;

	uint32_t getColorAttachmentCount() const;

	virtual std::vector<VkClearValue> getClearValues() const;

	VkSampleCountFlagBits getSampleCount() const;

	virtual uint32_t getRenderCount() const;

	void create();

	void recreate(VkExtent2D newExtent);

protected:
	RenderPass(Device *device, VkExtent2D extent, VkSampleCountFlagBits sampleCount);

	// possible formats of depth attachment
	const std::vector<VkFormat> DEPTH_FORMATS{
		VK_FORMAT_D32_SFLOAT,
		VK_FORMAT_D32_SFLOAT_S8_UINT,
		VK_FORMAT_D24_UNORM_S8_UINT
	};

	Device *device;

	VkRenderPass renderPass;

	std::vector<VkFramebuffer> framebuffers;

	VkExtent2D extent;

	VkFormat depthAttachmentFormat;

	VkSampleCountFlagBits sampleCount;

	std::vector<std::shared_ptr<Image>> attachments;

	virtual void createAttachments() = 0;

	virtual void createRenderPass() = 0;

	virtual void createFramebuffers() = 0;

	void addFramebuffer(std::vector<VkImageView> imageViews);

private:
	void cleanup();
};

enum RenderPassType
{
	DEPTH,
    GEOMETRY,
    SSAO,
    SSAO_BLUR,
    LIGHTING,
	FINAL
};

typedef std::unordered_map<RenderPassType, RenderPass*> RenderPassesMap;
