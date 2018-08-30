#include <set>
#include "Logger.h"
#include <array>

#include "Vulkan.h"

// public:

Vulkan::Vulkan(Window *pWindow)
{
	createInstance();
	createDebugCallback();
	createSurface(pWindow->window);
	
	pDevice = new Device(instance, surface, VALIDATION_LAYERS);
	pSwapChain = new SwapChain(pDevice, surface, pWindow->getExtent());

	pDescriptorSet = new DescriptorSet(pDevice);
	initDescriptorSet();

	pGraphicsPipeline = new GraphicsPipeline(pDevice, pSwapChain, pDescriptorSet->layout);

	initGraphicCommands();
}

Vulkan::~Vulkan()
{
	delete(pGraphicsPipeline);
	delete(pDescriptorSet);
	delete(pSwapChain);
	delete(pDevice);

	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyDebugReportCallbackEXT(instance, callback, nullptr);
	vkDestroyInstance(instance, nullptr);
}

// private:

void Vulkan::createInstance()
{
	// validation layers
	Logger::infoValidationLayers(ENABLE_VALIDATION_LAYERS);
	if (ENABLE_VALIDATION_LAYERS)
	{
		if (!checkInstanceLayerSupport(VALIDATION_LAYERS))
		{
			LOGGER_FATAL(Logger::VALIDATION_LAYERS_NOT_AVAILABLE);
		}
	}

	// required extenstions
	std::vector<const char *> extensions = getRequiredExtensions();
	if (!checkInstanceExtensionSupport(extensions))
	{
		LOGGER_FATAL(Logger::INSTANCE_EXTENSIONS_NOT_AVAILABLE);
	}

	// infoabout application for vulkan
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

	// info for vulkan instance
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

	// add validation layers if they are enabled
	if (ENABLE_VALIDATION_LAYERS)
	{
		createInfo.enabledLayerCount = VALIDATION_LAYERS.size();
		createInfo.ppEnabledLayerNames = VALIDATION_LAYERS.data();
	}

	VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
	if (result != VK_SUCCESS)
	{
		LOGGER_FATAL(Logger::FAILED_TO_CREATE_INSTANCE);
	}
}

bool Vulkan::checkInstanceLayerSupport(std::vector<const char*> requiredLayers)
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);  // get count

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());  // get layers

	std::set<std::string> requiredLayerSet(requiredLayers.begin(), requiredLayers.end());

	for (const auto& layer : availableLayers)
	{
		requiredLayerSet.erase(layer.layerName);
	}

	// empty if all required layers are supported by instance
	return requiredLayerSet.empty();
}

bool Vulkan::checkInstanceExtensionSupport(std::vector<const char*> requiredExtensions)
{
	uint32_t extensionCount;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);  // get count

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());  // get extensions

	std::set<std::string> requiredExtensionSet(requiredExtensions.begin(), requiredExtensions.end());

	for (const auto& layer : availableExtensions)
	{
		requiredExtensionSet.erase(layer.extensionName);
	}

	// empty if all required extensions are supported by instance
	return requiredExtensionSet.empty();
}

std::vector<const char*> Vulkan::getRequiredExtensions()
{
	std::vector<const char*> extensions;

	// glfw extensions, at least VK_KHR_surface
	unsigned int glfwExtensionCount = 0;
	const char **glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	for (int i = 0; i < glfwExtensionCount; i++)
	{
		extensions.push_back(glfwExtensions[i]);
	}

	// extension for validation layers callback
	if (ENABLE_VALIDATION_LAYERS)
	{
		extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	}

	return extensions;
}

VkResult Vulkan::vkCreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT * pCreateInfo, const VkAllocationCallbacks * pAllocator, VkDebugReportCallbackEXT * pCallback)
{
	auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(
		instance, "vkCreateDebugReportCallbackEXT"
	);

	if (func != nullptr)
	{
		return func(instance, pCreateInfo, pAllocator, pCallback);
	}
	else
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void Vulkan::vkDestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks * pAllocator)
{
	auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
	if (func != nullptr)
	{
		func(instance, callback, pAllocator);
	}
}

void Vulkan::createDebugCallback()
{
	// don't need callback if validation layers are not enabled
	if (!ENABLE_VALIDATION_LAYERS)
	{
		return;
	}

	VkDebugReportCallbackCreateInfoEXT createInfo = 
	{
		VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT,			// sType
		nullptr,															// pNext
		VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT,	// flags
		Logger::validationLayerCallback,									// pfnCallback
		nullptr																// pUserData
	};

	VkResult result = vkCreateDebugReportCallbackEXT(instance, &createInfo, nullptr, &callback);
	if (result != VK_SUCCESS)
	{
		LOGGER_FATAL(Logger::FAILED_TO_CREATE_CALLBACK);
	}
}

void Vulkan::createSurface(GLFWwindow *window)
{
	// glfw library create surface by it self
	VkResult result = glfwCreateWindowSurface(instance, window, nullptr, &surface);
	if (result != VK_SUCCESS)
	{
		LOGGER_FATAL(Logger::FAILED_TO_CREATE_SURFACE);
	}
}

void Vulkan::initDescriptorSet()
{
	const std::string EARTH_TEXTURE_PATH = File::getExeDir() + "textures/earth.jpg";
	const std::string EARTH_MODEL_PATH = File::getExeDir() + "models/sphere.obj";

	pEarthTexture = new TextureImage(pDevice, EARTH_TEXTURE_PATH);
	pEarthModel = new Model(pDevice, EARTH_MODEL_PATH);

	pDescriptorSet->addTexture(pEarthTexture);
	pDescriptorSet->addModel(pEarthModel);

	pDescriptorSet->update();  // save changes in descriptor set
}

void Vulkan::initGraphicCommands()
{
	// return old command buffers to pool
	if (!graphicCommands.empty())
	{
		vkFreeCommandBuffers(pDevice->device, pDevice->commandPool, graphicCommands.size(), graphicCommands.data());
	}

	graphicCommands.resize(pSwapChain->imageCount);

	VkCommandBufferAllocateInfo allocInfo{
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,	// sType;
		nullptr,										// pNext;
		pDevice->commandPool,							// commandPool;
		VK_COMMAND_BUFFER_LEVEL_PRIMARY,				// level;
		graphicCommands.size(),							// commandBufferCount;
	};

	VkResult result = vkAllocateCommandBuffers(pDevice->device, &allocInfo, graphicCommands.data());
	if (result != VK_SUCCESS)
	{
		LOGGER_FATAL(Logger::FAILED_TO_ALLOC_COMMAND_BUFFERS);
	}

	// clear values for each frame
	std::array<VkClearValue, 2> clearValues = {};
	clearValues[0].color = backgroundColor;
	clearValues[1].depthStencil = { 1, 0 };

	// render area for each frame
	VkRect2D renderArea{
		{ 0, 0 },			// offset
		pSwapChain->extent	// extent
	};

	for (int i = 0; i < graphicCommands.size(); i++)
	{
		VkCommandBufferBeginInfo beginInfo{
			VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,	// sType;
			nullptr,										// pNext;
			VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,	// flags;
			nullptr,										// pInheritanceInfo;
		};

		result = vkBeginCommandBuffer(graphicCommands[i], &beginInfo);
		if (result != VK_SUCCESS)
		{
			LOGGER_FATAL(Logger::FAILED_TO_BEGIN_COMMAND_BUFFER);
		}

		VkRenderPassBeginInfo renderPassBeginInfo{
			VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,	// sType;
			nullptr,									// pNext;
			pGraphicsPipeline->renderpass,				// renderPass;
			pGraphicsPipeline->framebuffers[i],			// framebuffer;
			renderArea,									// renderArea;
			clearValues.size(),							// clearValueCount;
			clearValues.data()							// pClearValues;
		};

		vkCmdBeginRenderPass(graphicCommands[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(graphicCommands[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pGraphicsPipeline->pipeline);

		pDescriptorSet->bind(graphicCommands[i], pGraphicsPipeline->layout);
		pDescriptorSet->drawModels(graphicCommands[i]);

		vkCmdEndRenderPass(graphicCommands[i]);

		result = vkEndCommandBuffer(graphicCommands[i]);
		if (result != VK_SUCCESS)
		{
			LOGGER_FATAL(Logger::FAILED_TO_END_COMMAND_BUFFER);
		}
	}
}





