#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>

#include "Camera.h"
#include "Space.h"

// public:

Camera::Camera(Device *device, VkExtent2D extent) : extent(extent)
{
	attributes.position = glm::vec3(0.0f, 0.0f, 0.0f);
	attributes.forward = glm::vec3(0.0f, 0.0f, 1.0f);
	attributes.up = glm::vec3(0.0f, -1.0f, 0.0f);
	attributes.fov = 45.0f;
	attributes.speed = 2.0f;
	attributes.sensitivity = 0.15f;
	attributes.nearPlane = 0.1f;
	attributes.farPlane = 100.0f;

	initAngles();
	initSpaceBuffer(device);
}

Camera::Camera(Device *device, VkExtent2D extent, Attributes attributes) : extent(extent), attributes(attributes)
{
	projectionMatrix = createProjectionMatrix();

	initAngles();
	initSpaceBuffer(device);
}

Camera::~Camera()
{
	delete(spaceBuffer);
}

glm::vec3 Camera::getPos() const
{
	return attributes.position;
}

glm::vec3 Camera::getTarget() const
{
	return attributes.forward + attributes.position;
}

glm::vec3 Camera::getUp() const
{
	return attributes.up;
}

Buffer* Camera::getSpaceBuffer() const
{
	return spaceBuffer;
}

glm::vec2 Camera::getCenter() const
{
	return glm::vec2(extent.width / 2, extent.height / 2);
}

float Camera::getNearPlane() const
{
	return attributes.nearPlane;
}

float Camera::getFarPlane() const
{
	return attributes.farPlane;
}

glm::mat4 Camera::getViewMatrix() const
{
	return lookAt(getPos(), getTarget(), getUp());
}

glm::mat4 Camera::getProjectionMatrix() const
{
	return projectionMatrix;
}

void Camera::move(float deltaSec)
{
	const float distance = attributes.speed * deltaSec;

	glm::vec3 direction = attributes.forward * float(movement.forward);

	glm::vec3 right = glm::cross(attributes.forward, attributes.up);
	right = glm::normalize(right);
	direction += right * float(movement.right);

	direction += attributes.up * float(movement.up);

	if (length(direction) != 0.0f)
	{
		direction = normalize(direction);
	}
	attributes.position += direction * distance;
}

void Camera::rotate(float deltaX, float deltaY)
{
	const float maxDelta = 100.0f;
	const float vertAngleLimit = 90.0f;

	deltaX = abs(deltaX) < maxDelta ? deltaX : maxDelta * deltaX / abs(deltaX);
	angleH += (deltaX * attributes.sensitivity);

	deltaY = abs(deltaY) < maxDelta ? deltaY : maxDelta * deltaY / abs(deltaY);
	angleV += (deltaY * attributes.sensitivity);
	// set vertical angle limits: -vertAngleLimit and vertAngleLimit degrees
	angleV = abs(angleV) > vertAngleLimit ? vertAngleLimit * angleV / abs(angleV) : angleV;

	const glm::vec3 vAxis{ 0.0f, 1.0f, 0.0f };

	// rotate the view by the horizontal angle
	glm::vec3 view(0.0f, 0.0f, 1.0f);
	view = glm::rotate(view, glm::radians(angleH), vAxis);
	view = normalize(view);

	// rotate the view by the vertical angle
	glm::vec3 hAxis = cross(view, vAxis);
	hAxis = normalize(hAxis);
	view = glm::rotate(view, glm::radians(angleV), hAxis);

	// save changes
	attributes.forward = normalize(view);
	attributes.up = cross(attributes.forward, hAxis);
	attributes.up = normalize(attributes.up);
}

void Camera::setExtent(VkExtent2D extent)
{
	this->extent = extent;

	projectionMatrix = createProjectionMatrix();

	updateSpace();
}

void Camera::setMovement(Movement movement)
{
	this->movement = movement;
}

void Camera::updateSpace() const
{
	Space space{ getViewMatrix(), projectionMatrix };
	spaceBuffer->updateData(&space, sizeof(space), 0);
}

// private:

void Camera::initAngles()
{
	glm::vec3 horizontal{ attributes.forward.x, 0.0f, attributes.forward.z };
	horizontal = normalize(horizontal);

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
	angleV = glm::degrees(glm::asin(attributes.forward.y));
}

void Camera::initSpaceBuffer(Device *device)
{
	spaceBuffer = new Buffer(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(Space));
    updateSpace();
}

glm::mat4 Camera::createProjectionMatrix() const
{
	const float aspect = extent.width / float(extent.height);

	glm::mat4 proj = glm::perspective(glm::radians(attributes.fov), aspect, attributes.nearPlane, attributes.farPlane);
	proj[1][1] *= -1;

	return proj;
}
