#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>

#include "Camera.h"

// public:

Camera::Camera(Device *pDevice, VkExtent2D extent)
{
	pos = glm::vec3(0.0f, 0.0f, 0.0f);
	forward = glm::vec3(0.0f, 0.0f, 1.0f);
	up = glm::vec3(0.0f, -1.0f, 0.0f);
	this->extent = extent;

	initAngles();
	initViewProj(pDevice);
}

Camera::Camera(Device *pDevice, glm::vec3 pos, glm::vec3 target, glm::vec3 up, VkExtent2D extent)
{
	this->pos = pos;
	this->forward = glm::normalize(target);
	this->up = glm::normalize(up);
	this->extent = extent;

	initAngles();
	initViewProj(pDevice);
}

Camera::~Camera()
{
	delete(pViewProjBuffer);
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

Buffer * Camera::getViewProjBuffer() const
{
	return pViewProjBuffer;
}

glm::vec2 Camera::getCenter() const
{
	return glm::vec2(extent.width / 2, extent.height / 2);
}

void Camera::move(float deltaSec)
{
	const float DISTANCE = SPEED * deltaSec;

	glm::vec3 direction = forward * (float)movement.forward;

	glm::vec3 right = glm::cross(forward, up);
	right = glm::normalize(right);
	direction += right * (float)movement.right;

	direction += up * (float)movement.up;

	if (glm::length(direction) != 0.0f)
	{
		direction = glm::normalize(direction);
	}
	pos += direction * DISTANCE;
}

void Camera::rotate(float deltaX, float deltaY)
{
	const float MAX_DELTA = 100.0f;
	const float VERT_ANGLE_LIMIT = 90.0f;

	deltaX = abs(deltaX) < MAX_DELTA ? deltaX : MAX_DELTA * deltaX / abs(deltaX);
	angleH += (deltaX * SENSITIVITY);

	deltaY = abs(deltaY) < MAX_DELTA ? deltaY : MAX_DELTA * deltaY / abs(deltaY);
	angleV += (deltaY * SENSITIVITY);
	// set vertical angle limits: -VERT_ANGLE_LIMIT and VERT_ANGLE_LIMIT degrees
	angleV = abs(angleV) > VERT_ANGLE_LIMIT ? VERT_ANGLE_LIMIT * angleV / abs(angleV) : angleV;

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

void Camera::setExtent(VkExtent2D extent)
{
	this->extent = extent;

	viewProj.projection = getProjectionMatrix();
	pViewProjBuffer->updateData(&viewProj.projection, sizeof(viewProj.projection), offsetof(ViewProjMatrices, projection));
}

void Camera::updateView()
{
	viewProj.view = getViewMatrix();
	pViewProjBuffer->updateData(&viewProj.view, sizeof(viewProj.view), offsetof(ViewProjMatrices, view));
}


// private:

void Camera::initAngles()
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

void Camera::initViewProj(Device *pDevice)
{
	viewProj.view = getViewMatrix();
	viewProj.projection = getProjectionMatrix();

	pViewProjBuffer = new Buffer(pDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_SHADER_STAGE_VERTEX_BIT, sizeof(viewProj));
	pViewProjBuffer->updateData(&viewProj, sizeof(viewProj), 0);
}

glm::mat4 Camera::getViewMatrix() const
{
	return glm::lookAt(getPos(), getTarget(), getUp());
}

glm::mat4 Camera::getProjectionMatrix() const
{
	const float viewAngle = 45.0f;
	const float aspect = extent.width / (float)extent.height;
	const float zNear = 0.1f;
	const float zFar = 10000.0f;

	return glm::perspective(glm::radians(viewAngle), aspect, zNear, zFar);
}
