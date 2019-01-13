#pragma once

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include "Engine.h"

class Window
{
public:
	enum Mode
	{
		WINDOWED,
		BORDERLESS,
		FULLSCREEN
	};

	Window(int width, int height, Mode mode);

	~Window();

	void setUserPointer(void *pointer) const;

	HWND getHWnd() const;

	VkExtent2D getClientExtent() const;

	void mainLoop() const;

private:
	enum Key
	{
		MOVE_FORWARD = GLFW_KEY_W,
		MOVE_LEFT = GLFW_KEY_A,
		MOVE_BACKWARD = GLFW_KEY_S,
		MOVE_RIGHT = GLFW_KEY_D,
		MOVE_UP = GLFW_KEY_SPACE,
		MOVE_DOWN = GLFW_KEY_LEFT_CONTROL,
	};

	GLFWwindow *window;

	void controlCamera(Camera *camera) const;

	Camera::Direction getDirection(int positiveKey, int negativeKey) const;

	bool isPressed(int key) const;

	static Engine* getEngine(GLFWwindow *window);

	static void framebufferSizeCallback(GLFWwindow *window, int width, int height);
};

