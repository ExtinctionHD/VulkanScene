#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include "Device.h"

class ShaderModule
{
public:
	ShaderModule(Device *device, const std::string &path, VkShaderStageFlagBits stage);

	ShaderModule(
		Device *device,
        const std::string &path, 
		VkShaderStageFlagBits stage, 
		std::vector<VkSpecializationMapEntry> entries,
	    std::vector<const void*> data);

	~ShaderModule();

	VkShaderStageFlagBits getStage() const;

	VkShaderModule getModule() const;

	VkSpecializationInfo* getSpecializationInfo() const;

private:
	Device *device;

	VkShaderStageFlagBits stage;

	VkShaderModule module{};

	std::vector<VkSpecializationMapEntry> entries;

	void *data{};

	VkSpecializationInfo *pSpecializationInfo{};
};

