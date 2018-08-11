#pragma once

#include <GLFW/glfw3.h>

class Window
{
public:
	int width = 1280;
	int height = 720;

	void init();  // initialize library and create window

	void mainLoop();

	operator GLFWwindow*();  // return window descriptor instead of this
private:
	GLFWwindow * window;  // window descriptor
};

