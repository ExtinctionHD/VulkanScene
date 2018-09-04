#include "Camera.h"

// public:

Camera::Camera()
{
	pos = glm::vec3(0.0f, 0.0f, 0.0f);
	forward = glm::vec3(0.0f, 0.0f, 1.0f);
	up = glm::vec3(0.0f, -1.0f, 0.0f);
}

Camera::Camera(glm::vec3 pos, glm::vec3 target, glm::vec3 up)
{
	this->pos = pos;
	this->forward = target;
	this->up = up;
}

Camera::~Camera()
{

}

glm::vec3 Camera::getPos() const
{
	return pos;
}

glm::vec3 Camera::getTarget() const
{
	return forward + pos;
}

glm::vec3 Camera::getUp() const
{
	return up;
}

bool Camera::onKeyPress(int key)
{
	switch (key)
	{
	case GLFW_KEY_W:
		pos += (forward * STEP_SIZE);
		return true;

	case GLFW_KEY_S:
		pos -= (forward * STEP_SIZE);
		return true;

	case GLFW_KEY_A:
		{
			glm::vec3 left = glm::cross(up, forward);
			left = glm::normalize(left);
			pos += (left * STEP_SIZE);
			return true;
		}

	case GLFW_KEY_D:
		{
			glm::vec3 right = glm::cross(forward, up);
			right = glm::normalize(right);
			pos += (right * STEP_SIZE);
			return true;
		}

	case GLFW_KEY_LEFT_SHIFT:
		pos += (up * STEP_SIZE);
		return true;

	case GLFW_KEY_LEFT_CONTROL:
		pos -= (up * STEP_SIZE);
		return true;

	default:
		return false;
	}
}

// private: