#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>

#include "Camera.h"

// public:

Camera::Camera(VkExtent2D extent)
{
	pos = glm::vec3(0.0f, 0.0f, 0.0f);
	forward = glm::vec3(0.0f, 0.0f, 1.0f);
	up = glm::vec3(0.0f, -1.0f, 0.0f);
	this->extent = extent;

	init();
}

Camera::Camera(glm::vec3 pos, glm::vec3 target, glm::vec3 up, VkExtent2D extent)
{
	this->pos = pos;
	this->forward = glm::normalize(target);
	this->up = glm::normalize(up);
	this->extent = extent;

	init();
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

void Camera::onKeyPress(int key)
{
	switch (key)
	{
	case GLFW_KEY_W:
		pos += (forward * STEP_SIZE);
		break;

	case GLFW_KEY_S:
		pos -= (forward * STEP_SIZE);
		break;

	case GLFW_KEY_A:
		{
			glm::vec3 left = glm::cross(up, forward);
			left = glm::normalize(left);
			pos += (left * STEP_SIZE);
			break;
		}

	case GLFW_KEY_D:
		{
			glm::vec3 right = glm::cross(forward, up);
			right = glm::normalize(right);
			pos += (right * STEP_SIZE);
			break;
		}

	case GLFW_KEY_LEFT_SHIFT:
		pos += (up * STEP_SIZE);
		break;

	case GLFW_KEY_LEFT_CONTROL:
		pos -= (up * STEP_SIZE);
		break;

	default:
		break;
	}
}

void Camera::onMouseMove(float x, float y)
{
	float deltaX = x - getCenter().x;
	float deltaY = y - getCenter().y;

	const float MAX_DELTA = 10.0f;
	deltaX = abs(deltaX) < MAX_DELTA ? deltaX : MAX_DELTA * deltaX / abs(deltaX);
	deltaY = abs(deltaY) < MAX_DELTA ? deltaY : MAX_DELTA * deltaY / abs(deltaY);

	angleH += (deltaX * SENSITIVITY);
	angleV += (deltaY * SENSITIVITY);

	const glm::vec3 vAxis{ 0.0f, 1.0f, 0.0f };

	// rotate the view by the horizontal angle
	glm::vec3 view(1.0f, 0.0f, 0.0f);
	view = glm::rotate(view, angleH, vAxis);
	view = glm::normalize(view);

	// rotate the view by the vertical angle
	glm::vec3 hAxis = glm::cross(view, vAxis);
	hAxis = glm::normalize(hAxis);
	view = glm::rotate(view, angleV, hAxis);

	// save changes
	forward = glm::normalize(view);
	up = glm::cross(forward, hAxis);
	up = glm::normalize(up);
}

// private:

void Camera::init()
{
	glm::vec3 horizontal{ forward.x, 0.0f, forward.z };
	horizontal = glm::normalize(horizontal);

	// horizontal camera angle
	if (horizontal.z >= 0.0f)
	{
		if (horizontal.x >= 0.0f)
		{
			angleH = 360.0f - glm::degrees(glm::asin(horizontal.z));
		}
		else
		{
			angleH = 180.0f + glm::degrees(glm::asin(horizontal.z));
		}
	}
	else
	{

		if (horizontal.x >= 0.0f)
		{
			angleH = glm::degrees(glm::asin(-horizontal.z));
		}
		else
		{
			angleH = 90.0f + glm::degrees(glm::asin(-horizontal.z));
		}
	}

	// vertical camera angle
	angleV = -glm::degrees(glm::asin(forward.y));
}

glm::vec2 Camera::getCenter() const
{
	return glm::vec2(extent.width / 2.0f, extent.height / 2.0f);
}