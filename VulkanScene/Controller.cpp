#include "Controller.h"

Controller::Controller(Camera *pCamera)
{
	this->pCamera = pCamera;
}

void Controller::controlCamera(float deltaSec)
{
	POINT pos;
	GetCursorPos(&pos);

	const glm::vec2 center = pCamera->getCenter();
	pCamera->rotate(pos.x - center.x, pos.y - center.y);
	SetCursorPos(center.x, center.y);

	setCameraMovement();

	pCamera->move(deltaSec);
	pCamera->updateSpace();
}

void Controller::setCameraMovement()
{
    if (isPressed(MOVE_FORWARD))
    {
		pCamera->movement.forward = Camera::Direction::POSITIVE;
    }
	else if (isPressed(MOVE_BACK))
	{
		pCamera->movement.forward = Camera::Direction::NEGATIVE;
	}
	else
	{
		pCamera->movement.forward = Camera::Direction::NONE;
	}

	if (isPressed(MOVE_RIGHT))
	{
		pCamera->movement.right = Camera::Direction::POSITIVE;
	}
	else if (isPressed(MOVE_LEFT))
	{
		pCamera->movement.right = Camera::Direction::NEGATIVE;
	}
	else
	{
		pCamera->movement.right = Camera::Direction::NONE;
	}

	if (isPressed(MOVE_UP))
	{
		pCamera->movement.up = Camera::Direction::POSITIVE;
	}
	else if (isPressed(MOVE_DOWN))
	{
		pCamera->movement.up = Camera::Direction::NEGATIVE;
	}
	else
	{
		pCamera->movement.up = Camera::Direction::NONE;
	}
}

bool Controller::isPressed(Key key)
{
	SHORT keyState = GetAsyncKeyState(key);

	return 1 << 15 & keyState;
}
