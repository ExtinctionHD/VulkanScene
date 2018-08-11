#include <string>
#include <algorithm>

#include "Logger.h"

// public:

const std::string Logger::VALIDATION_LAYERS_NOT_AVAILABLE = "Required validation layers not available";
const std::string Logger::INSTANCE_EXTENSIONS_NOT_AVAILABLE = "Required instance extensions not available";
const std::string Logger::FAILED_TO_CREATE_INSTANCE = "Failed to create vulkan instance";
const std::string Logger::FAILED_TO_CREATE_CALLBACK = "Failed to create validation layer debug callback";
const std::string Logger::FAILED_TO_CREATE_SURFACE = "Failed to create window surface.";
const std::string Logger::NO_GPU_WITH_VULKAN_SUPPORT = "GPU with vulkan support not found.";
const std::string Logger::NO_SUITABLE_GPU = "Suitable GPU not found";

void Logger::infoValidationLayers(bool enabled)
{
	printInfo();

	if (enabled)
	{
		std::cout << "Validation layers enabled." << std::endl;
	}
	else
	{
		std::cout << "Validation layers not enabled." << std::endl;
	}
}

void Logger::fatal(std::string message, std::string file, int line)
{
	throw std::runtime_error("\nFatal: " + message + ". In " + getFilename(file) + " at line " + std::to_string(line));
}

VKAPI_ATTR VkBool32 VKAPI_CALL Logger::validationLayerCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location, int32_t code, const char * layerPrefix, const char * msg, void * userData)
{
	std::cerr << "Validation layer: " << msg << "." << std::endl;

	return VK_FALSE;
}

// private:

void Logger::printInfo()
{
	std::cout << "Info: ";
}

std::string Logger::getFilename(std::string path)
{
	std::replace(path.begin(), path.end(), '\\', '/');

	return path.substr(path.find_last_of('/') + 1);
}
