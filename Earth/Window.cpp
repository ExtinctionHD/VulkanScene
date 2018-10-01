#include "Window.h"

// public:

Window::Window(int width, int height)
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	window = glfwCreateWindow(width, height, "Vulkan API", nullptr, nullptr);

	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
	glfwSetKeyCallback(window, keyCallback);
	glfwSetCursorPosCallback(window, cursorPosCallback);

	setCursorInCenter(window);
}

void Window::mainLoop()
{
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		// getVulkanPointer(window)->drawFrame();
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

void Window::framebufferSizeCallback(GLFWwindow *window, int width, int height)
{
	// case of minimization
	while (width == 0 || height == 0)
	{
		glfwGetFramebufferSize(window, &width, &height);
		glfwWaitEvents();
	}

	VkExtent2D extent{
		(uint32_t)width,
		(uint32_t)height
	};

	// getVulkanPointer(window)->resize(extent);
}

void Window::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
	// getVulkanPointer(window)->onKeyAction(key, action);
}

void Window::cursorPosCallback(GLFWwindow * window, double x, double y)
{
	// getVulkanPointer(window)->onMouseMove(x, y);

	// setCursorInCenter(window);
}

void Window::setCursorInCenter(GLFWwindow * window)
{
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);

	glfwSetCursorPos(window, width / 2.0, height / 2.0);
}

