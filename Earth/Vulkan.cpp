#include <set>
#include <GLFW/glfw3.h>

#include "Logger.h"

#include "Vulkan.h"

void Vulkan::init()
{
	createInstance();
}

void Vulkan::createInstance()
{
	// слои проверок
	Logger::infoValidationLayers(enableValidationLayers);
	if (enableValidationLayers)
	{
		if (!checkInstanceLayerSupport(validationLayers))
		{
			LOGGER_FATAL(Logger::VALIDATION_LAYERS_NOT_AVAILABLE);
		}
	}

	// требуемые расширения
	std::vector<const char *> extensions = getRequiredExtensions();
	if (!checkInstanceExtensionSupport(extensions))
	{
		LOGGER_FATAL(Logger::INSTANCE_EXTENSIONS_NOT_AVAILABLE);
	}

	// информация о приложении
	VkApplicationInfo appInfo =
	{
		VK_STRUCTURE_TYPE_APPLICATION_INFO,	// sType
		nullptr,							// pNext
		"Earth",							// pApplicationName
		VK_MAKE_VERSION(1, 0, 0),			// applicationVersion
		"No Engine",						// pEngineName
		VK_MAKE_VERSION(1, 0, 0),			// engineVersion
		VK_API_VERSION_1_0					// apiVersion
	};

	// информация для создания экземпляра Vulkan
	VkInstanceCreateInfo createInfo =
	{
		VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,	// sType
		nullptr,								// pNext
		0,										// flags
		&appInfo,								// pApplicationInfo
		0,										// enabledLayerCount
		nullptr,								// ppEnabledLayerNames
		extensions.size(),						// enabledExtensionCount
		extensions.data()						// ppEnabledExtensionNames
	};

	// если включены слои проверок добавляем их в структуру
	if (enableValidationLayers)
	{
		createInfo.enabledLayerCount = validationLayers.size();
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}

	// создание экземпляра Vulkan
	VkResult result = vkCreateInstance(&createInfo, nullptr, instance.replace());
	if (result != VK_SUCCESS)
	{
		LOGGER_FATAL(Logger::FAILED_TO_CREATE_INSTANCE);
	}
}

bool Vulkan::checkInstanceLayerSupport(std::vector<const char*> requiredLayers)
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	std::set<std::string> requiredLayersSet(requiredLayers.begin(), requiredLayers.end());

	for (const auto& layer : availableLayers)
	{
		requiredLayersSet.erase(layer.layerName);
	}

	return requiredLayersSet.empty();
}

bool Vulkan::checkInstanceExtensionSupport(std::vector<const char*> requiredExtensions)
{
	uint32_t extensionCount;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensionsSet(requiredExtensions.begin(), requiredExtensions.end());

	for (const auto& layer : availableExtensions)
	{
		requiredExtensionsSet.erase(layer.extensionName);
	}

	return requiredExtensionsSet.empty();
}

std::vector<const char*> Vulkan::getRequiredExtensions()
{
	std::vector<const char*> extensions;

	unsigned int glfwExtensionCount = 0;
	const char **glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	for (int i = 0; i < glfwExtensionCount; i++)
	{
		extensions.push_back(glfwExtensions[i]);
	}

	if (enableValidationLayers)
	{
		extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	}

	return extensions;
}

