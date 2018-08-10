#include <string>
#include <algorithm>

#include "Logger.h"

const std::string Logger::VALIDATION_LAYERS_NOT_AVAILABLE = "Required validation layers not available";
const std::string Logger::INSTANCE_EXTENSIONS_NOT_AVAILABLE = "Required instance extensions not available";
const std::string Logger::FAILED_TO_CREATE_INSTANCE = "Failed to create vulkan instance";

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

void Logger::printInfo()
{
	std::cout << "Info: ";
}

std::string Logger::getFilename(std::string path)
{
	std::replace(path.begin(), path.end(), '\\', '/');

	return path.substr(path.find_last_of('/') + 1);
}
