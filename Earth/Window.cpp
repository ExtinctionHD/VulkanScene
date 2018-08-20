#include "Window.h"

// public:

Window::Window()
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	window = glfwCreateWindow(baseWidth, baseHeight, "Vulkan API", nullptr, nullptr);
}

void Window::mainLoop()
{
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
	}
}

VkExtent2D Window::getExtent() const
{
	VkExtent2D extent;

	glfwGetWindowSize(window, (int *)&extent.width, (int *)&extent.height);

	return extent;
}

Window::~Window()
{
	glfwDestroyWindow(window);

	glfwTerminate();
}

