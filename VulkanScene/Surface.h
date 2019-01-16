#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include "GLFW/glfw3native.h"

class Surface
{
public:
	Surface(VkInstance instance, HWND hWnd);

	Surface(VkInstance instance, GLFWwindow *window);

	~Surface();

	VkSurfaceKHR get() const;

private:
	VkSurfaceKHR surface;

	VkInstance instance;

	void createSurface(HWND hWnd);
};

