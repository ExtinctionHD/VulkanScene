#include "Controller.h"
#include <ctime>

Controller::Controller(Camera *pCamera)
{
	this->pCamera = pCamera;
}

Controller::~Controller()
{
}

void Controller::controlCamera(float deltaSec)
{
	POINT pos;
	GetCursorPos(&pos);

	glm::vec2 center = pCamera->getCenter();

	pCamera->rotateCamera(pos.x - center.x, pos.y - center.y);

	SetCursorPos(center.x, center.y);

	pCamera->moveCamera(deltaSec);
}

void Controller::keyDownCallback(int key)
{
	switch (key)
	{
	case Keys::moveForward:
		pCamera->movement.forward = Camera::Direction::positive;
		break;
	case Keys::moveBack:
		pCamera->movement.forward = Camera::Direction::negative;
		break;
	case Keys::moveRight:
		pCamera->movement.right = Camera::Direction::positive;
		break;
	case Keys::moveLeft:
		pCamera->movement.right = Camera::Direction::negative;
		break;
	case Keys::moveUp:
		pCamera->movement.up = Camera::Direction::positive;
		break;
	case Keys::moveDown:
		pCamera->movement.up = Camera::Direction::negative;
		break;
	default:
		break;
	}
}

void Controller::keyUpCallback(int key)
{
	switch (key)
	{
	case Keys::moveForward:
		pCamera->movement.forward = Camera::Direction::none;
		break;
	case Keys::moveBack:
		pCamera->movement.forward = Camera::Direction::none;
		break;
	case Keys::moveRight:
		pCamera->movement.right = Camera::Direction::none;
		break;
	case Keys::moveLeft:
		pCamera->movement.right = Camera::Direction::none;
		break;
	case Keys::moveUp:
		pCamera->movement.up = Camera::Direction::none;
		break;
	case Keys::moveDown:
		pCamera->movement.up = Camera::Direction::none;
		break;
	default:
		break;
	}
}
