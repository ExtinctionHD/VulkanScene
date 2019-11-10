#include "Window.h"

int main(int argc, char *argv[])
{
	const Settings settings{
		VK_SAMPLE_COUNT_4_BIT,
		4096,
		"Assets/FullScene.json",
	};

	auto window = Window(1920, 1080, Window::BORDERLESS);
	auto engine = Engine(
		window.getHWnd(),
		window.getClientExtent(),
        settings);

	window.setUserPointer(&engine);
    window.mainLoop();

	return 0;
}