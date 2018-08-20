#pragma once

#include <iostream>
#include <vulkan/vulkan.h>

// log fatal errors with filename and current line
#define LOGGER_FATAL(msg)														\
{																				\
	std::string message = (msg);												\
	Logger::fatal(Logger::VALIDATION_LAYERS_NOT_AVAILABLE, __FILE__, __LINE__);	\
}

class Logger
{
public:
	static const std::string VALIDATION_LAYERS_NOT_AVAILABLE;
	static const std::string INSTANCE_EXTENSIONS_NOT_AVAILABLE;
	static const std::string FAILED_TO_CREATE_INSTANCE;
	static const std::string FAILED_TO_CREATE_CALLBACK;
	static const std::string FAILED_TO_CREATE_SURFACE;
	static const std::string NO_GPU_WITH_VULKAN_SUPPORT;
	static const std::string NO_SUITABLE_GPU;
	static const std::string FAILED_TO_CREATE_LOGICAL_DEVICE;
	static const std::string FAILED_TO_CREATE_SWAPCHAIN;
	static const std::string FAILED_TO_CREATE_IMAGE;
	static const std::string FAILED_TO_ALLOC_IMAGE_MEMORY;
	static const std::string FAILED_TO_FIND_MEMORY_TYPE;
	static const std::string FAILED_TO_CREATE_IMAGE_VIEW;
	static const std::string FAILED_TO_FIND_SUPPORTED_FORMAT;
	static const std::string FAILED_TO_CREATE_RENDER_PASS;
	

	static void infoValidationLayers(bool enabled);  // log inforamation about validation layers

	static void fatal(std::string message, std::string file, int line);  // log fatal errors

	// log messages from vulkan validation layers
	static VKAPI_ATTR VkBool32 VKAPI_CALL validationLayerCallback(
		VkDebugReportFlagsEXT flags,
		VkDebugReportObjectTypeEXT objType,
		uint64_t obj,
		size_t location,
		int32_t code,
		const char *layerPrefix,
		const char *msg,
		void *userData
	);

private:
	static void printInfo();

	static std::string getFilename(std::string path);  // get only filename from full path
};

