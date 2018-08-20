#include "Application.h"

// public:

Application::Application()
{
	pWindow = new Window();
	pVulkan = new Vulkan(pWindow);
}

Application::~Application()
{
	delete(pVulkan);
	delete(pWindow);
}

void Application::run()
{
	pWindow->mainLoop();
}