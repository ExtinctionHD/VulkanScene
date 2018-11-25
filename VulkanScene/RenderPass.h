#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include "Device.h"
#include <unordered_map>
#include "Image.h"

class RenderPass
{
public:
    virtual ~RenderPass();

	VkRenderPass getRenderPass() const;

	std::vector<VkFramebuffer> getFramebuffers() const;

	VkExtent2D getExtent() const;

	virtual uint32_t getColorAttachmentCount() const = 0;

	virtual std::vector<VkClearValue> getClearValues() const;

	VkSampleCountFlagBits getSampleCount() const;

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

	VkSampleCountFlagBits sampleCount;

	std::vector<Image*> attachments{};

	virtual void createAttachments() = 0;

	virtual void createRenderPass() = 0;

	virtual void createFramebuffers() = 0;

private:
	void cleanup();
};


enum RenderPassType
{
	DEPTH,
    GEOMETRY,
    LIGHTING,
	FINAL
};

typedef std::unordered_map<RenderPassType, RenderPass*> RenderPassesMap;
