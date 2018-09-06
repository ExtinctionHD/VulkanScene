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

void Camera::onKeyDown(int key)
{
	switch (key)
	{
	case GLFW_KEY_W:
		movement.forward = Direction::positive;
		break;

	case GLFW_KEY_S:
		movement.forward = Direction::negative;
		break;

	case GLFW_KEY_A:
	{
		movement.right = Direction::negative;
		break;
	}

	case GLFW_KEY_D:
	{
		movement.right = Direction::positive;
		break;
	}

	case GLFW_KEY_SPACE:
		movement.up = Direction::positive;
		break;

	case GLFW_KEY_LEFT_CONTROL:
		movement.up = Direction::negative;
		break;

	default:
		break;
	}
}

void Camera::onKeyUp(int key)
{
	switch (key)
	{
	case GLFW_KEY_W:
		movement.forward = Direction::none;
		break;

	case GLFW_KEY_S:
		movement.forward = Direction::none;
		break;

	case GLFW_KEY_A:
	{
		movement.right = Direction::none;
		break;
	}

	case GLFW_KEY_D:
	{
		movement.right = Direction::none;
		break;
	}

	case GLFW_KEY_SPACE:
		movement.up = Direction::none;
		break;

	case GLFW_KEY_LEFT_CONTROL:
		movement.up = Direction::none;
		break;

	default:
		break;
	}
}

void Camera::moveCamera(float deltaSec)
{
	const float DISTANCE = SPEED * deltaSec;

	pos += forward * (float)movement.forward * DISTANCE;

	glm::vec3 right = glm::cross(forward, up);
	right = glm::normalize(right);
	pos += right * (float)movement.right * DISTANCE;

	pos += up * (float)movement.up * DISTANCE;
}

void Camera::onMouseMove(float x, float y)
{
	const float MAX_DELTA = 100.0f;
	const float VERT_ANGLE_LIMIT = 90.0f;

	float deltaX = x - getCenter().x;
	deltaX = abs(deltaX) < MAX_DELTA ? deltaX : MAX_DELTA * deltaX / abs(deltaX);
	angleH += (deltaX * SENSITIVITY);

	float deltaY = y - getCenter().y;
	deltaY = abs(deltaY) < MAX_DELTA ? deltaY : MAX_DELTA * deltaY / abs(deltaY);
	angleV += (deltaY * SENSITIVITY);
	// set vertical angle limits: -VERT_ANGLE_LIMIT and VERT_ANGLE_LIMIT degrees
	angleV = abs(deltaY) > VERT_ANGLE_LIMIT ? VERT_ANGLE_LIMIT * angleV / abs(angleV) : angleV;

	const glm::vec3 vAxis{ 0.0f, 1.0f, 0.0f };

	// rotate the view by the horizontal angle
	glm::vec3 view(0.0f, 0.0f, 1.0f);
	view = glm::rotate(view, glm::radians(angleH), vAxis);
	view = glm::normalize(view);

	// rotate the view by the vertical angle
	glm::vec3 hAxis = glm::cross(view, vAxis);
	hAxis = glm::normalize(hAxis);
	view = glm::rotate(view, glm::radians(angleV), hAxis);

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
	if (horizontal.x >= 0.0f)
	{
		if (horizontal.z >= 0.0f)
		{
			// first quarter
			angleH = 360.0f - glm::degrees(glm::asin(horizontal.x));
		}
		else
		{
			// second quarter
			angleH = 180.0f + glm::degrees(glm::asin(horizontal.x));
		}
	}
	else
	{
		if (horizontal.z >= 0.0f)
		{
			// third quarter
			angleH = glm::degrees(glm::asin(-horizontal.x));
		}
		else
		{
			// fourth quarter
			angleH = 90.0f + glm::degrees(glm::asin(-horizontal.x));
		}
	}

	// vertical camera angle
	angleV = -glm::degrees(glm::asin(forward.y));
}

glm::vec2 Camera::getCenter() const
{
	return glm::vec2(extent.width / 2.0f, extent.height / 2.0f);
}