#pragma once

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include "Vulkan.h"

class Window
{
public:
	GLFWwindow * window;  // window descriptor

	Window();  // initialize library and create window

	~Window();  // detroy window

	void mainLoop(Vulkan *pVulkan);

	VkExtent2D getExtent() const;  // window width and height in VKExtent2D structure

private:
	// initial window extent
	const int baseWidth = 1280;
	const int baseHeight = 720;
};

