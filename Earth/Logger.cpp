#include <string>
#include <algorithm>
#include "File.h"

#include "Logger.h"

// public:

const std::string Logger::VALIDATION_LAYERS_NOT_AVAILABLE = "Required validation layers not available";
const std::string Logger::INSTANCE_EXTENSIONS_NOT_AVAILABLE = "Required instance extensions not available";
const std::string Logger::FAILED_TO_CREATE_INSTANCE = "Failed to create vulkan instance";
const std::string Logger::FAILED_TO_CREATE_CALLBACK = "Failed to create validation layer debug callback";
const std::string Logger::FAILED_TO_CREATE_SURFACE = "Failed to create window surface.";
const std::string Logger::NO_GPU_WITH_VULKAN_SUPPORT = "GPU with vulkan support not found.";
const std::string Logger::NO_SUITABLE_GPU = "Suitable GPU not found";
const std::string Logger::FAILED_TO_CREATE_LOGICAL_DEVICE = "Failed to create logical device";
const std::string Logger::FAILED_TO_CREATE_SWAPCHAIN = "Failed to create SwapChain object";
const std::string Logger::FAILED_TO_CREATE_IMAGE = "Failed to create image";
const std::string Logger::FAILED_TO_ALLOC_IMAGE_MEMORY = "Failed to allocate image memory";
const std::string Logger::FAILED_TO_FIND_MEMORY_TYPE = "Failed to find suitable memory type";
const std::string Logger::FAILED_TO_CREATE_IMAGE_VIEW = "Failed to create image view";
const std::string Logger::FAILED_TO_FIND_SUPPORTED_FORMAT = "Failed to find supported format from requsted formats";
const std::string Logger::FAILED_TO_CREATE_RENDER_PASS = "Failed to create render pass";
const std::string Logger::FAILED_TO_CREATE_DS_LAYOUT = "Failed to create descriptor set layout";
const std::string Logger::FAILED_TO_CREATE_PIPELINE_LAYOUT = "Failed to create pipeline layout";
const std::string Logger::FAILED_TO_CREATE_GRAPHICS_PIPELINE = "Failed to create graphics pipeline";
const std::string Logger::FAILED_TO_CREATE_COMMAND_POOL = "Failed to create command pool";
const std::string Logger::UNSUPPORTED_LAYOUT_TRANSITION = "Unsupported image layout transition";
const std::string Logger::FAILED_TO_CREATE_FRAMEBUFFER = "Failed to create framebuffer";
const std::string Logger::IMAGE_FORMAT_DOES_NOT_SUPPORT_LINEAR_BLITTING = "This image format does not support linear blitting";
const std::string Logger::FAILED_TO_CREATE_TEXTURE_SAMPLER = "Failed to create texture sampler";

void Logger::printInfo(std::string message)
{
	std::cout << "INFO:  " << message << std::endl;
}

void Logger::infoValidationLayers(bool enabled)
{
	if (enabled)
	{
		printInfo("Validation layers enabled.");
	}
	else
	{
		printInfo("Validation layers not enabled.");
	}
}

void Logger::fatal(std::string message, std::string file, int line)
{
	throw std::runtime_error("\nFATAL: " + message + ". In " + File::getFilename(file) + " at line " + std::to_string(line));
}

VKAPI_ATTR VkBool32 VKAPI_CALL Logger::validationLayerCallback(
	VkDebugReportFlagsEXT flags,
	VkDebugReportObjectTypeEXT objType,
	uint64_t obj, 
	size_t location, 
	int32_t code, 
	const char * layerPrefix, 
	const char * msg, 
	void * userData
)
{
	std::cerr << "Validation layer: " << msg << "." << std::endl;

	return VK_FALSE;
}

std::string Logger::getFileOpeningErrMsg(std::string filename)
{
	return "Failed to open file: " + filename;
}

std::string Logger::getShaderCreatingErrMsg(std::string filename)
{
	return "Failed to create shader module from file: " + filename;
}

std::string Logger::getTextureLoadingErrMsg(std::string filename)
{
	return "Failed to load texture image from file: " + filename;
}

std::string Logger::getModelLoadingErrMsg(std::string err)
{
	return formatModelErr(err);
}

// private:

std::string Logger::formatModelErr(std::string err)
{
	err = err.substr(0, err.length() - 1);

	size_t index = err.find_first_of("\n", 0);
	while (index != std::string::npos)
	{
		err.insert(index + 1, "       ");
		size_t offset = index + 1;
		index = err.find_first_of("\n", offset);
	}

	std::replace(err.begin(), err.end(), '\\', '/');

	return err;
}
