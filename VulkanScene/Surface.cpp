#include <iostream>
#include <cassert>

#include "Surface.h"

// public:

Surface::Surface(VkInstance instance, HWND hWnd)
{
	this->instance = instance;

	createSurface(hWnd);
}

Surface::Surface(VkInstance instance, GLFWwindow *window)
{
	this->instance = instance;

	glfwCreateWindowSurface(instance, window, nullptr, &surface);

	assert(surface);
}

Surface::~Surface()
{
	vkDestroySurfaceKHR(instance, surface, nullptr);
}

VkSurfaceKHR Surface::get() const
{
	return surface;
}

// private:

void Surface::createSurface(HWND hWnd)
{
	VkWin32SurfaceCreateInfoKHR createInfo{
		VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
		nullptr,
		0,
		GetModuleHandle(nullptr),
		hWnd
	};

    const VkResult result = vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr, &surface);
	assert(result == VK_SUCCESS);
}
