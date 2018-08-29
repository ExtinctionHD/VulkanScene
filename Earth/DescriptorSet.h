#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include "Buffer.h"
#include "TextureImage.h"

// provides a pipeline with resources: buffers and textures
class DescriptorSet
{
public:
	DescriptorSet(VkDevice device);
	~DescriptorSet();

	VkDescriptorSetLayout layout;

private:
	// device that provide descriptor set
	VkDevice device;

	std::vector<Buffer> uniformBuffers;

	std::vector<TextureImage> textures;

	void createLayout();
};

