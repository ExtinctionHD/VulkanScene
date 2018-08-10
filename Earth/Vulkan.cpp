#include "Log.h"

#include "Vulkan.h"

void Vulkan::init()
{
	createInstance();
}

void Vulkan::createInstance()
{
	Log::validationLayers(enableValidationLayers);
}
