#include "Application.h"

// public:

void Application::run()
{
	window.init();
	vulkan.init();

	window.mainLoop();
}