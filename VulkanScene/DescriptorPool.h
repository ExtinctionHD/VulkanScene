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

	// returns set of descriptors and create the layout of this descriptor set
	VkDescriptorSet getDescriptorSet(
		std::vector<Buffer*> buffers,
        std::vector<VkShaderStageFlagBits> buffersShaderStages,
		std::vector<TextureImage*> textures,
		std::vector<VkShaderStageFlagBits> texturesShaderStages,
		bool createLayout,
		VkDescriptorSetLayout& layout
	);

private:
	Device *pDevice;

	VkDescriptorPool pool{};

	VkDescriptorSetLayout createDescriptorSetLayout(
		std::vector<VkShaderStageFlagBits> buffersShaderStages,
		std::vector<VkShaderStageFlagBits> texturesShaderStages
	);
};

struct DescriptorStruct
{
	VkDescriptorSet set;
	VkDescriptorSetLayout layout;
};

