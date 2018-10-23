#include <iostream>
#include <cassert>

#include "Surface.h"

// public:

Surface::Surface(VkInstance instance, HINSTANCE hInstance, HWND hWnd)
{
	this->instance = instance;

	createSurface(hInstance, hWnd);
}

Surface::~Surface()
{
	vkDestroySurfaceKHR(instance, surface, nullptr);
}

VkSurfaceKHR Surface::getSurface() const
{
	return surface;
}

void Surface::createSurface(HINSTANCE hInstance, HWND hWnd)
{
	VkWin32SurfaceCreateInfoKHR createInfo{
		VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,	// sType;
		nullptr,											// pNext;
		0,													// flags;
		hInstance,											// hinstance;
		hWnd												// hwnd;
	};

	VkResult result = vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr, &surface);
	assert(result == VK_SUCCESS);
}
