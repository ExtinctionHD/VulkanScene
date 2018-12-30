#pragma once

#include <vulkan/vulkan.h>
#include <vector>

class ShaderModule
{
public:
	ShaderModule(VkDevice device, std::string path, VkShaderStageFlagBits stage);

	ShaderModule(
		VkDevice device, 
		std::string path, 
		VkShaderStageFlagBits stage, 
		std::vector<VkSpecializationMapEntry> entries,
	    const std::vector<const void*> &data
	);

	~ShaderModule();

	VkShaderStageFlagBits getStage() const;

	VkShaderModule getModule() const;

	VkSpecializationInfo* getSpecializationInfo() const;

private:
	// device that provide shader module
	VkDevice device;

	VkShaderStageFlagBits stage;

	VkShaderModule module{};

	std::vector<VkSpecializationMapEntry> entries;

	void *pData{};

	VkSpecializationInfo *pSpecializationInfo{};
};

