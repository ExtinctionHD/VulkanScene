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
		std::ofstream logFile("VL.log");
		std::cerr.rdbuf(logFile.rdbuf());

		pWindow = new Window(hInstance, 1280, 720);
		pVulkan = new Vulkan(pWindow->getHInstance(), pWindow->getHWnd(), pWindow->getClientExtent());
		pWindow->setUserPointer(pVulkan);
		pWindow->mainLoop();
	}
	catch (const std::exception& ex)
	{
		MessageBox(pWindow->getHWnd(), ex.what(), "Error", MB_ICONERROR);
	}

	delete(pWindow);
	delete(pVulkan);
}