#include "Window.h"
#include <atlstr.h>
#include <iostream>

#define CORRECT_ARGC 9

Settings getSettings(LPCWSTR lpCmdLine);

int APIENTRY wWinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow
)
{
    // Settings settings = getSettings(lpCmdLine);

	Settings settings{
		VK_SAMPLE_COUNT_2_BIT,
		4096,
		50.0f,
		true,
		"Clouds",
		{ false, false, true, false }
	};

	auto pWindow = new Window(hInstance, 1280, 720);

	auto pEngine = new Engine(
		pWindow->getHInstance(), 
		pWindow->getHWnd(), 
		pWindow->getClientExtent(),
        settings
	);

    std::cout << "" << std::endl;

    pWindow->setUserPointer(pEngine);
    int result = pWindow->mainLoop();

	delete(pWindow);
	delete(pEngine);

	return result;
}

Settings getSettings(LPCWSTR lpCmdLine)
{
    int argc;
    LPWSTR *argv = CommandLineToArgvW(lpCmdLine, &argc);
    assert(argc == CORRECT_ARGC);

    std::wstring ws(argv[4]);
    std::string lightingScheme = std::string(ws.begin(), ws.end());

    std::vector<bool> modelsExistence{
        bool(_wtoi(argv[5])),
        bool(_wtoi(argv[6])),
        bool(_wtoi(argv[7])),
        bool(_wtoi(argv[8]))
    };

    return Settings{
        VkSampleCountFlagBits(_wtoi(argv[0])),
        uint32_t(_wtoi(argv[1])),
        float(_wtof(argv[2])),
        bool(_wtoi(argv[3])),
        lightingScheme,
        modelsExistence
    };
}