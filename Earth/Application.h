#pragma once

#include "Window.h"
#include "Vulkan.h"

class Application
{
public:
	Application();

	~Application();

	void run();

private:
	Window *pWindow;

	Vulkan *pVulkan;
};

