#include <iostream>
#include <fstream>
#include "Window.h"

int APIENTRY wWinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow
)
{
	Window *pWindow = new Window(hInstance, 1280, 720);
	Vulkan *pVulkan = new Vulkan(pWindow->getHInstance(), pWindow->getHWnd(), pWindow->getClientExtent());

	pWindow->setUserPointer(pVulkan);
	pWindow->mainLoop();

	delete(pWindow);
	delete(pVulkan);
}