#pragma once

#include <vulkan/vulkan.h>

class ShaderModule
{
public:
	ShaderModule(VkDevice device, std::string filename, VkShaderStageFlagBits stage);
	~ShaderModule();

	VkShaderStageFlagBits stage;

	VkShaderModule module{};

private:
	// device that provide shader module
	VkDevice device;
};

