#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include "Buffer.h"

class Camera
{
public:
	enum Direction
	{
		NEGATIVE = -1,
		NONE = 0,
		POSITIVE = 1
	};

	struct Movement
	{
		Direction forward = NONE;
		Direction right = NONE;
		Direction up = NONE;
	};

	struct Attributes
	{
		glm::vec3 position;
		glm::vec3 forward;
		glm::vec3 up;
		float fov;
		float speed;
		float sensitivity;
		float nearPlane;
		float farPlane;
	};

    // creates camera located in (0.0, 0.0, 0.0), locking at z axis, with 45.0 degrees field of view
	Camera(Device *device, VkExtent2D extent);

	Camera(Device *device, VkExtent2D extent, Attributes attributes);

	~Camera();

	glm::vec3 getPos() const;

	glm::vec3 getTarget() const;

	glm::vec3 getUp() const;

	Buffer* getSpaceBuffer() const;

	glm::vec2 getCenter() const;

	float getNearPlane() const;

	float getFarPlane() const;

	glm::mat4 getViewMatrix() const;

	glm::mat4 getProjectionMatrix() const;

	void move(float deltaSec);

	void rotate(float deltaX, float deltaY);

	void setExtent(VkExtent2D extent);

	void setMovement(Movement movement);

	void updateSpace() const;

private:
	VkExtent2D extent;

	Attributes attributes;

	Movement movement;

	// horizontal angle
	// angle 0.0f directed towards z axis
	float angleH;

	// vertical angle
	float angleV;

	glm::mat4 projectionMatrix;

	Buffer *spaceBuffer;

	void initAngles();

	void initSpaceBuffer(Device *device);

	glm::mat4 createProjectionMatrix() const;
};

