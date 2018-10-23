#pragma once

#include "Camera.h"
#include <Windows.h>

class Controller
{
public:
	Controller(Camera *pCamera);

	enum Keys
	{
		moveForward = 0x57,		// W
		moveLeft = 0x41,		// A
		moveBack = 0x53,		// S
		moveRight = 0x44,		// D
		moveUp = VK_SPACE,		// Space
		moveDown = VK_CONTROL,	// Ctrl
	};

	// moves and rotates camera
	void controlCamera(float deltaSec);

	void keyDownCallback(int key);

	void keyUpCallback(int key);

private:
	Camera *pCamera;
};

