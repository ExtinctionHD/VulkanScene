#pragma once

#include <vulkan/vulkan.h>
#include "Device.h"
#include "Buffer.h"
#include "TextureImage.h"

// provides pool of resource descriptors
class DescriptorPool
{
public:
	DescriptorPool(Device *pDevice, uint32_t bufferCount, uint32_t texturesCount, uint32_t setCount);
	~DescriptorPool();

	VkDescriptorSetLayout createDescriptorSetLayout(
		std::vector<VkShaderStageFlags> buffersShaderStages,
		std::vector<VkShaderStageFlags> texturesShaderStages
	) const;

	// returns set of descriptors and create the layout of this descriptor set
	VkDescriptorSet getDescriptorSet(
		VkDescriptorSetLayout layout
	) const;

	void updateDescriptorSet(
		VkDescriptorSet set,
		std::vector<Buffer*> buffers,
		std::vector<TextureImage*> textures
	) const;

private:
	Device *pDevice;

	VkDescriptorPool pool{};
};

struct DescriptorStruct
{
	VkDescriptorSetLayout layout;
	VkDescriptorSet set;
};

