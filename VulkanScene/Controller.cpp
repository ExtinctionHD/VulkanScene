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

	pCamera->move(deltaSec);

	pCamera->updateSpace();
}

void Controller::keyDownCallback(int key)
{
	switch (key)
	{
	case Keys::MOVE_FORWARD:
		pCamera->movement.forward = Camera::Direction::POSITIVE;
		break;
	case Keys::MOVE_BACK:
		pCamera->movement.forward = Camera::Direction::NEGATIVE;
		break;
	case Keys::MOVE_RIGHT:
		pCamera->movement.right = Camera::Direction::POSITIVE;
		break;
	case Keys::MOVE_LEFT:
		pCamera->movement.right = Camera::Direction::NEGATIVE;
		break;
	case Keys::MOVE_UP:
		pCamera->movement.up = Camera::Direction::POSITIVE;
		break;
	case Keys::MOVE_DOWN:
		pCamera->movement.up = Camera::Direction::NEGATIVE;
		break;
	default:
		break;
	}
}

void Controller::keyUpCallback(int key)
{
	switch (key)
	{
	case Keys::MOVE_FORWARD:
		pCamera->movement.forward = Camera::Direction::NONE;
		break;
	case Keys::MOVE_BACK:
		pCamera->movement.forward = Camera::Direction::NONE;
		break;
	case Keys::MOVE_RIGHT:
		pCamera->movement.right = Camera::Direction::NONE;
		break;
	case Keys::MOVE_LEFT:
		pCamera->movement.right = Camera::Direction::NONE;
		break;
	case Keys::MOVE_UP:
		pCamera->movement.up = Camera::Direction::NONE;
		break;
	case Keys::MOVE_DOWN:
		pCamera->movement.up = Camera::Direction::NONE;
		break;
	default:
		break;
	}
}
