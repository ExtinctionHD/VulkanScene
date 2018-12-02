#include "Window.h"

int APIENTRY wWinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow
)
{
	auto pWindow = new Window(hInstance, 1280, 720);

	auto pVulkan = new Vulkan(
		pWindow->getHInstance(), 
		pWindow->getHWnd(), 
		pWindow->getClientExtent(),
        VkSampleCountFlagBits(4),
		true,
        4096,
		40.0f,
		File::getExeDir() + "assets/Noon.dat"
	);

	pWindow->setUserPointer(pVulkan);
	pWindow->mainLoop();

	delete(pWindow);
	delete(pVulkan);

	return 0;
}
