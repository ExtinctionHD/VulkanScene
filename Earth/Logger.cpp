#include <string>
#include <algorithm>
#include "File.h"

#include "Logger.h"

// public:

using namespace std;

const string Logger::VALIDATION_LAYERS_NOT_AVAILABLE = "Required validation layers not available";
const string Logger::INSTANCE_EXTENSIONS_NOT_AVAILABLE = "Required instance extensions not available";
const string Logger::FAILED_TO_CREATE_INSTANCE = "Failed to create vulkan instance";
const string Logger::FAILED_TO_CREATE_CALLBACK = "Failed to create validation layer debug callback";
const string Logger::FAILED_TO_CREATE_SURFACE = "Failed to create window surface.";
const string Logger::NO_GPU_WITH_VULKAN_SUPPORT = "GPU with vulkan support not found.";
const string Logger::NO_SUITABLE_GPU = "Suitable GPU not found";
const string Logger::FAILED_TO_CREATE_LOGICAL_DEVICE = "Failed to create logical device";
const string Logger::FAILED_TO_CREATE_SWAPCHAIN = "Failed to create SwapChain object";
const string Logger::FAILED_TO_CREATE_IMAGE = "Failed to create image";
const string Logger::FAILED_TO_ALLOC_IMAGE_MEMORY = "Failed to allocate image memory";
const string Logger::FAILED_TO_FIND_MEMORY_TYPE = "Failed to find suitable memory type";
const string Logger::FAILED_TO_CREATE_IMAGE_VIEW = "Failed to create image view";
const string Logger::FAILED_TO_FIND_SUPPORTED_FORMAT = "Failed to find supported format from requsted formats";
const string Logger::FAILED_TO_CREATE_RENDER_PASS = "Failed to create render pass";
const string Logger::FAILED_TO_CREATE_DS_LAYOUT = "Failed to create descriptor set layout";
const string Logger::FAILED_TO_CREATE_PIPELINE_LAYOUT = "Failed to create pipeline layout";
const string Logger::FAILED_TO_CREATE_GRAPHICS_PIPELINE = "Failed to create graphics pipeline";
const string Logger::FAILED_TO_CREATE_COMMAND_POOL = "Failed to create command pool";
const string Logger::UNSUPPORTED_LAYOUT_TRANSITION = "Unsupported image layout transition";
const string Logger::FAILED_TO_CREATE_FRAMEBUFFER = "Failed to create framebuffer";
const string Logger::IMAGE_FORMAT_DOES_NOT_SUPPORT_LINEAR_BLITTING = "This image format does not support linear blitting";
const string Logger::FAILED_TO_CREATE_TEXTURE_SAMPLER = "Failed to create texture sampler";
const string Logger::FAILED_TO_CREATE_BUFFER = "Failed to create buffer";
const string Logger::FAILED_TO_ALLOC_BUFFER_MEMORY = "Failed to allocate buffer memory";
const string Logger::FAILED_TO_CREATE_DESCRIPTOR_POOL = "Failed to create descriptor pool";
const string Logger::FAILED_TO_ALLOC_DESCRIPTOR_SET = "Failed to allocate descriptor set";
const string Logger::FAILED_TO_ALLOC_COMMAND_BUFFERS = "Failed to allocate command buffers";
const string Logger::FAILED_TO_BEGIN_COMMAND_BUFFER = "Failed to begin command buffers";
const string Logger::FAILED_TO_END_COMMAND_BUFFER = "Failed to end command buffers";
const string Logger::FAILED_TO_CREATE_SEMAPHORE = "Failed to create semaphore";
const string Logger::FAILED_TO_ACQUIRE_NEXT_FRAME = "Failed to acquire next frame";
const string Logger::FAILED_TO_SUBMIT_COMMANDS = "Failed to submit commands";
const string Logger::FAILED_TO_PRESENT_FRAME = "Failed to present frame";

void Logger::printInfo(string message)
{
	cout << "INFO:  " << message << endl;
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

void Logger::fatal(string message, string file, int line)
{
	throw runtime_error("\nFATAL: " + message + ". In " + File::getFilename(file) + " at line " + to_string(line));
}

string Logger::getFileOpeningErrMsg(string filename)
{
	return "Failed to open file: " + filename;
}

string Logger::getShaderCreatingErrMsg(string filename)
{
	return "Failed to create shader module from file: " + filename;
}

string Logger::getTextureLoadingErrMsg(string filename)
{
	return "Failed to load texture image from file: " + filename;
}

string Logger::getModelLoadingErrMsg(string err)
{
	return formatModelErr(err);
}

// private:

string Logger::formatModelErr(string err)
{
	err = err.substr(0, err.length() - 1);

	size_t index = err.find_first_of("\n", 0);
	while (index != string::npos)
	{
		err.insert(index + 1, "       ");
		size_t offset = index + 1;
		index = err.find_first_of("\n", offset);
	}

	replace(err.begin(), err.end(), '\\', '/');

	return err;
}
