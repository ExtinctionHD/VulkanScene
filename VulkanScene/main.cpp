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
	Window *pWindow = nullptr;
	Vulkan *pVulkan = nullptr;

	try
	{
		pWindow = new Window(hInstance, 1280, 720);
		pVulkan = new Vulkan(pWindow->getHInstance(), pWindow->getHWnd(), pWindow->getClientExtent());
		pWindow->setUserPointer(pVulkan);
		pWindow->mainLoop();
	}
	catch (const std::exception& ex)
	{
		MessageBox(pWindow->getHWnd(), std::string("FATAL | " + std::string(ex.what())).c_str(), "Error", MB_ICONERROR);
	}

	delete(pWindow);
	delete(pVulkan);
}