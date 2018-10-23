#pragma once

#include <Windows.h>
#include <vulkan/vulkan.h>
#include "Vulkan.h"

class Window
{
public:
	Window();

	Window(HINSTANCE hInstance, int width, int height);

	void setUserPointer(void *pointer);

	VkExtent2D getClientExtent() const;

	static VkExtent2D getClientExtent(HWND hWnd);

	HINSTANCE getHInstance() const;

	HWND getHWnd() const;

	int mainLoop();

private:
	const std::string WINDOW_CLASS = "Vulkan API";
	const std::string WINDOW_TITLE = "Vulkan scene";

	HINSTANCE hInstance;	// instance handler
	HWND hWnd;				// window handler

	ATOM registerWindowClass();

	// create window
	void createWindow(int width, int height);

	void showWindow();

	// function of window messages processing
	static LRESULT CALLBACK wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};

