#pragma once

#include "Camera.h"
#include <Windows.h>

class Controller
{
public:
	Controller();
	~Controller();

	struct Keys
	{
		int moveForward = 0x57;		// W
		int moveLeft = 0x41;		// A
		int moveBack = 0x53;		// S
		int moveRight = 0x44;		// D
		int moveUp = VK_SPACE;		// Space
		int moveDown = VK_LCONTROL; // Ctrl
	} keys;

	// moves and rotates camera
	void controlCamera(Camera *pCamera, float deltaSec);
};

