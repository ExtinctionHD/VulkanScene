#pragma once

#include <vulkan/vulkan.h>
#include "Device.h"
#include "Buffer.h"
#include "TextureImage.h"

class DescriptorPool
{
public:
	DescriptorPool(Device *pDevice, uint32_t bufferCount, uint32_t texturesCount);
	~DescriptorPool();

	VkDescriptorSet getDescriptorSet(
		std::vector<Buffer*> buffers,
		std::vector<TextureImage*> textures,
		bool createLayout,
		VkDescriptorSetLayout layout
	);

private:
	Device *pDevice;

	VkDescriptorPool pool;

	VkDescriptorSetLayout createDescriptorSetLayout(std::vector<Buffer*> buffers, std::vector<TextureImage*> textures);
};

