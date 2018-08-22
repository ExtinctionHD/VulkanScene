#pragma once

#include <vulkan/vulkan.h>

class ShaderModule
{
public:
	ShaderModule(VkDevice device, std::string filename);
	~ShaderModule();

	operator VkShaderModule();

private:
	// device that provide shader module
	VkDevice device;

	VkShaderModule shaderModule;
};

