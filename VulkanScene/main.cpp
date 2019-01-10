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
		40.0f,
		true,
		"Assets/SimpleScene.json",
	};

	auto window = Window(hInstance, 1280, 720);

	auto engine = Engine(
		window.getHInstance(),
		window.getHWnd(),
		window.getClientExtent(),
        settings
	);

    std::cout << "" << std::endl;

	window.setUserPointer(&engine);
    int result = window.mainLoop();

	return result;
}

Settings getSettings(LPCWSTR lpCmdLine)
{
    int argc;
    LPWSTR *argv = CommandLineToArgvW(lpCmdLine, &argc);
    assert(argc == CORRECT_ARGC);

    std::wstring ws(argv[4]);
    std::string scenePath = std::string(ws.begin(), ws.end());

    return Settings{
        VkSampleCountFlagBits(_wtoi(argv[0])),
        uint32_t(_wtoi(argv[1])),
        float(_wtof(argv[2])),
        bool(_wtoi(argv[3])),
		scenePath
    };
}