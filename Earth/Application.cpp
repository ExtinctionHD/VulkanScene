#include "Application.h"

void Application::run()
{
	window.init();
	vulkan.init();

	window.mainLoop();
}