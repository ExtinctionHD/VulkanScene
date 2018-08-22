#pragma once

#include <vulkan/vulkan.h>

class DescriptorSet
{
public:
	DescriptorSet(VkDevice device);
	~DescriptorSet();

	VkDescriptorSetLayout layout;

private:
	// device that provide descriptor set
	VkDevice device;

	void createLayout();
};

