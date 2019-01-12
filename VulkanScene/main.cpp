#include "Window.h"

int main(int argc, char *argv[])
{
	Settings settings{
		VK_SAMPLE_COUNT_2_BIT,
		4096,
		40.0f,
		true,
		"Assets/SimpleScene.json",
	};

	auto window = Window(1920, 1080, true);

	auto engine = Engine(
		window.getHWnd(),
		window.getClientExtent(),
        settings
	);

	window.setUserPointer(&engine);
    window.mainLoop();

	return 0;
}