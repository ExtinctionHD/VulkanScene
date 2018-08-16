#pragma once

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

class Window
{
public:
	void init();  // initialize library and create window

	void mainLoop();

	VkExtent2D getExtent() const;  // window width and height in VKExtent2D structure

	operator GLFWwindow*();  // return window descriptor instead of this
private:
	// initial window extent
	const int baseWidth = 1280;
	const int baseHeight = 720;

	GLFWwindow *window;  // window descriptor
};

