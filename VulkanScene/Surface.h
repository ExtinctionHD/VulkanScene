#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include <Windows.h>

class Surface
{
public:
	Surface(VkInstance instance, HINSTANCE hInstance, HWND hWnd);
	~Surface();

	VkSurfaceKHR getSurface() const;

private:
	VkSurfaceKHR surface;

	VkInstance instance;

	void createSurface(HINSTANCE hInstance, HWND hWnd);
};

