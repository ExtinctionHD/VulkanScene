#include "Window.h"

// public:

Window::Window(int width, int height, Mode mode)
{
	assert(glfwInit());

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	GLFWmonitor *monitor = nullptr;
	switch (mode)
	{
	case WINDOWED:
		break;
	case BORDERLESS_WINDOWED:
		glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
		glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
		break;
	case FULLSCREEN:
		monitor = glfwGetPrimaryMonitor();
		break;
	default: ;
	}

	window = glfwCreateWindow(width, height, "Vulkan API", monitor, nullptr);
	assert(window);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
}

Window::~Window()
{
	glfwDestroyWindow(window);
	glfwTerminate();
}

void Window::setUserPointer(void* pointer) const
{
	glfwSetWindowUserPointer(window, pointer);
}

HWND Window::getHWnd() const
{
	return glfwGetWin32Window(window);
}

VkExtent2D Window::getClientExtent() const
{
	VkExtent2D extent;

	glfwGetFramebufferSize(
		window, 
		reinterpret_cast<int *>(&extent.width), 
		reinterpret_cast<int *>(&extent.height)
	);

	return extent;
}

void Window::mainLoop() const
{
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		Engine *engine = getEngine(window);
		controlCamera(engine->getCamera());
		engine->drawFrame();
	}
}

// private:

void Window::controlCamera(Camera* camera) const
{
	static double prevX, prevY;
	double x, y;
	glfwGetCursorPos(window, &x, &y);
	camera->rotate(x - prevX, y - prevY);
	prevX = x;
	prevY = y;

	if (isPressed(MOVE_FORWARD))
	{
		camera->movement.forward = Camera::Direction::POSITIVE;
	}
	else if (isPressed(MOVE_BACK))
	{
		camera->movement.forward = Camera::Direction::NEGATIVE;
	}
	else
	{
		camera->movement.forward = Camera::Direction::NONE;
	}

	if (isPressed(MOVE_RIGHT))
	{
		camera->movement.right = Camera::Direction::POSITIVE;
	}
	else if (isPressed(MOVE_LEFT))
	{
		camera->movement.right = Camera::Direction::NEGATIVE;
	}
	else
	{
		camera->movement.right = Camera::Direction::NONE;
	}

	if (isPressed(MOVE_UP))
	{
		camera->movement.up = Camera::Direction::POSITIVE;
	}
	else if (isPressed(MOVE_DOWN))
	{
		camera->movement.up = Camera::Direction::NEGATIVE;
	}
	else
	{
		camera->movement.up = Camera::Direction::NONE;
	}
}

bool Window::isPressed(int key) const
{
	return glfwGetKey(window, key) == GLFW_PRESS;
}

Engine* Window::getEngine(GLFWwindow* window)
{
	return reinterpret_cast<Engine*>(glfwGetWindowUserPointer(window));
}

void Window::framebufferSizeCallback(GLFWwindow *window, int width, int height)
{
	Engine *engine = getEngine(window);
	if (!engine)
	{
		return;
	}

	if (width == 0 || height == 0)
	{
		engine->minimized = true;
	}
	else
	{
		engine->minimized = false;
	}

	VkExtent2D extent{
		uint32_t(width),
		uint32_t(height)
	};

	getEngine(window)->resize(extent);
}
