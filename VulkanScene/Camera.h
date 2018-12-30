#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include "Buffer.h"

// provides camera attributes
// and movement functions
class Camera
{
public:
	struct Attributes
	{
		glm::vec3 position;
		glm::vec3 forward;
		glm::vec3 up;
		float fov;
		float speed;
		float sensitivity;
	};

    // creates camera located in (0.0, 0.0, 0.0), locking at z axis, with 45.0 degrees field of view
	Camera(Device *pDevice, VkExtent2D extent);

	Camera(Device *pDevice, VkExtent2D extent, Attributes attributes);

	~Camera();

	enum Direction
	{
		NEGATIVE = -1,
		NONE = 0,
		POSITIVE = 1
	};

	// asix movement states
	struct Movement
	{
		Direction forward = NONE;
		Direction right = NONE;
		Direction up = NONE;
	} movement;

	glm::vec3 getPos() const;

	glm::vec3 getTarget() const;

	glm::vec3 getUp() const;

	Buffer* getSpaceBuffer() const;

	glm::vec2 getCenter() const;

	void move(float deltaSec);

	void rotate(float deltaX, float deltaY);

	void setExtent(VkExtent2D extent);

	void updateSpace();

private:
	glm::vec3 pos{};

	glm::vec3 forward{};

	glm::vec3 up{};

	VkExtent2D extent{};

	float fov;

	float speed;

	float sensitivity;

	// horizontal angle
	// angle 0.0f directed towards z axis
	float angleH{};

	// vertical angle
	float angleV{};

	// camera view and projection matrices
	Buffer *pSpaceBuffer{};

	// initializes camera angles
	void initAngles();

	void initSpaceBuffer(Device *pDevice);

	glm::mat4 getViewMatrix() const;

	glm::mat4 getProjectionMatrix() const;
};

