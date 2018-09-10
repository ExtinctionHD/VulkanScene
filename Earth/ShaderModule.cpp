#include <vector>
#include "File.h"
#include "Logger.h"

#include "ShaderModule.h"

// public:

ShaderModule::ShaderModule(VkDevice device, std::string filename, VkShaderStageFlagBits stage)
{
	this->device = device;
	this->stage = stage;

	std::vector<char> code = File::getFileBytes(filename);

	VkShaderModuleCreateInfo createInfo{
		VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,	// sType;
		nullptr,										// pNext;
		0,												// flags;
		code.size(),									// codeSize;
		(uint32_t*)code.data()							// pCode;
	};

	VkResult result = vkCreateShaderModule(device, &createInfo, nullptr, &module);
	if (result != VK_SUCCESS)
	{
		LOGGER_FATAL(Logger::getShaderCreatingErrMsg(filename));
	}
}

ShaderModule::~ShaderModule()
{
	vkDestroyShaderModule(device, module, nullptr);
}
