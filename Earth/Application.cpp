#include "Application.h"

// public:

Application::Application()
{
	pWindow = new Window();
	pVulkan = new Vulkan(pWindow->window, pWindow->getFrameExtent());
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