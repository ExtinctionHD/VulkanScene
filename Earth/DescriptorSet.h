#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include "Buffer.h"
#include "TextureImage.h"

// provides a pipeline with resources: uniform buffers and textures
// deletes all resources during destroying
class DescriptorSet
{
public:
	DescriptorSet(Device *pDevice);
	~DescriptorSet();

	// resources
	std::vector<Buffer*> uniformBuffers;
	std::vector<TextureImage*> textures;

	// layout of current resouces
	VkDescriptorSetLayout layout = VK_NULL_HANDLE;

	// updates layout, pool and set
	// call after resource change
	void update();

private:
	// device that provide descriptor set
	Device *pDevice;

	VkDescriptorPool pool = VK_NULL_HANDLE;

	VkDescriptorSet set = VK_NULL_HANDLE;

	// create layout of current resources
	void createLayout();

	// create pool of descriptors for current resources
	void createDescriptorPool();

	// create descriptor set of current resources
	void createDescriptorSet();

	// equals number of uniform buffers
	uint32_t getTextureBindingsOffset();

	void initUniformBuffers();

	void initTextures();
};

