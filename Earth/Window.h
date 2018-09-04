#pragma once

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include "Vulkan.h"

class Window
{
public:
	GLFWwindow *window;  // window descriptor

	Window(int width, int height);  // initialize library and create window

	~Window();  // detroy window

	void mainLoop();

	VkExtent2D getFrameExtent() const;  // window width and height in VKExtent2D structure

private:
	static Vulkan* getVulkanPointer(GLFWwindow *window);

	static void framebufferSizeCallback(GLFWwindow *window, int width, int height);
};

