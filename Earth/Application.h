#pragma once

#include "Window.h"
#include "Vulkan.h"

class Application
{
public:
	void run();

private:
	Window window;

	Vulkan vulkan;
};

