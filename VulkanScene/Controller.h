#pragma once

#include "Camera.h"
#include <Windows.h>

class Controller
{
public:
	Controller(Camera *pCamera);

	enum Key
	{
		MOVE_FORWARD = 0x57,    // W
		MOVE_LEFT = 0x41,       // A
		MOVE_BACK = 0x53,       // S
		MOVE_RIGHT = 0x44,      // D
		MOVE_UP = VK_SPACE,     // Space
		MOVE_DOWN = VK_CONTROL, // Ctrl
	};

	// moves and rotates camera
	void controlCamera(float deltaSec);

private:
	Camera *pCamera;

	void setCameraMovement();

	bool isPressed(Key key);
};

