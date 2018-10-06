#include "Controller.h"
#include <ctime>

Controller::Controller()
{
}

Controller::~Controller()
{
}

void Controller::controlCamera(Camera * pCamera, float deltaSec)
{

	POINT pos;
	GetCursorPos(&pos);
	pCamera->rotateCamera(glm::vec2(pos.x, pos.y));

	glm::vec2 center = pCamera->getCenter();
	SetCursorPos(center.x, center.y);

	if (GetAsyncKeyState(keys.moveForward) != 0)
	{
		pCamera->movement.forward = Camera::Direction::positive;
	}
	if (GetAsyncKeyState(keys.moveBack) != 0)
	{
		pCamera->movement.forward = Camera::Direction::negative;
	}

	if (GetAsyncKeyState(keys.moveRight) != 0)
	{
		pCamera->movement.right = Camera::Direction::positive;
	}
	if (GetAsyncKeyState(keys.moveLeft) != 0)
	{
		pCamera->movement.right = Camera::Direction::negative;
	}

	if (GetAsyncKeyState(keys.moveUp) != 0)
	{
		pCamera->movement.up = Camera::Direction::positive;
	}
	if (GetAsyncKeyState(keys.moveDown) != 0)
	{
		pCamera->movement.up = Camera::Direction::negative;
	}

	pCamera->moveCamera(deltaSec);
}
