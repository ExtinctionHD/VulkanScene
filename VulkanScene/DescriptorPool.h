#pragma once

#include <vulkan/vulkan.h>
#include "Device.h"
#include "Buffer.h"
#include "TextureImage.h"

class DescriptorPool
{
public:
	DescriptorPool(Device *device, uint32_t bufferCount, uint32_t textureCount, uint32_t setCount);

	~DescriptorPool();

	VkDescriptorSetLayout createDescriptorSetLayout(
		std::vector<VkShaderStageFlags> buffersShaderStages,
		std::vector<VkShaderStageFlags> texturesShaderStages) const;

	VkDescriptorSet getDescriptorSet(VkDescriptorSetLayout layout) const;

	void updateDescriptorSet(
		VkDescriptorSet set,
		std::vector<Buffer*> buffers,
		std::vector<TextureImage*> textures) const;

private:
	Device *device;

	VkDescriptorPool pool;
};

struct DescriptorStruct
{
	VkDescriptorSetLayout layout;
	VkDescriptorSet set;
};

