#pragma once

#include <GLFW/glfw3.h>

class Window
{
public:
	int width = 1280;
	int height = 720;

	void init();  // инициализация библиотеки и создание окна

	void mainLoop();  // цикл работы приложения

	operator GLFWwindow*();  // приведение к дескриптору окна
private:
	GLFWwindow * window;  // дескриптор окна
};

