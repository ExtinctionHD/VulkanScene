#include <vector>
#include "File.h"
#include <cassert>

#include "ShaderModule.h"

// public:

ShaderModule::ShaderModule(VkDevice device, std::string path, VkShaderStageFlagBits stage)
{
	this->device = device;
	this->stage = stage;

	std::vector<char> code = File::getBytes(path);

	VkShaderModuleCreateInfo createInfo{
		VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,    // sType;
		nullptr,                                        // pNext;
		0,                                              // flags;
		code.size(),									// codeSize;
		reinterpret_cast<uint32_t*>(code.data())		// pCode;
	};

	VkResult result = vkCreateShaderModule(device, &createInfo, nullptr, &module);
	assert(result == VK_SUCCESS);
}

ShaderModule::ShaderModule(
    VkDevice device,
    std::string path,
    VkShaderStageFlagBits stage,
    std::vector<VkSpecializationMapEntry> entries,
    const std::vector<const void*> &data
) : ShaderModule(device, path, stage)
{
	assert(entries.size() == data.size());

	this->entries = entries;

	VkSpecializationMapEntry lastEntry = *(--entries.end());
	size_t size = lastEntry.offset + lastEntry.size;

	pData = malloc(size);
    for (size_t i = 0; i < data.size(); i++)
    {
		memcpy(reinterpret_cast<char*>(pData) + entries[i].offset, data[i], entries[i].size);
    }

	pSpecializationInfo = new VkSpecializationInfo{
		uint32_t(entries.size()),
		this->entries.data(),
		size,
		this->pData
	};
}

ShaderModule::~ShaderModule()
{
	delete pSpecializationInfo;
	free(pData);

	vkDestroyShaderModule(device, module, nullptr);
}

VkShaderStageFlagBits ShaderModule::getStage() const
{
	return stage;
}

VkShaderModule ShaderModule::getModule() const
{
	return module;
}

VkSpecializationInfo* ShaderModule::getSpecializationInfo() const
{
	return pSpecializationInfo;
}
