#include "Window.h"

// public:

Window::Window()
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	window = glfwCreateWindow(baseWidth, baseHeight, "Vulkan API", nullptr, nullptr);
	glfwSetFramebufferSizeCallback(window, Window::framebufferSizeCallback);
}

void Window::mainLoop()
{
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		getVulkanPointer(window)->drawFrame();
	}
}

Window::~Window()
{
	glfwDestroyWindow(window);

	glfwTerminate();
}

VkExtent2D Window::getFrameExtent() const
{
	VkExtent2D extent;

	glfwGetFramebufferSize(window, (int *)&extent.width, (int *)&extent.height);

	return extent;
}

Vulkan * Window::getVulkanPointer(GLFWwindow *window)
{
	return reinterpret_cast<Vulkan*>(glfwGetWindowUserPointer(window));
}

void Window::framebufferSizeCallback(GLFWwindow * window, int width, int height)
{
	// case of minimization
	while (width == 0 || height == 0)
	{
		glfwGetFramebufferSize(window, &width, &height);
		glfwWaitEvents();
	}

	getVulkanPointer(window)->resize({ (uint32_t)width, (uint32_t)height });
}

