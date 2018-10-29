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
    // creates camera located in (0.0, 0.0, 0.0), locking at z axis, with 45.0 degrees field of view
	Camera(Device *pDevice, VkExtent2D extent);

	Camera(Device *pDevice, glm::vec3 pos, glm::vec3 forward, glm::vec3 up, VkExtent2D extent, float fov);

	~Camera();

	enum Direction
	{
		negative = -1,
		none = 0,
		positive = 1
	};

	// asix movement states
	struct Movement
	{
		Direction forward = Direction::none;
		Direction right = Direction::none;
		Direction up = Direction::none;
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
	// step of camera movement
	const float SPEED = 1.0f;

	// camera rotation sensitivity
	const float SENSITIVITY = 0.1f;

	// position of camera
	glm::vec3 pos{};

	// vector of camera direction
	glm::vec3 forward{};

	// up vector of camera 
	glm::vec3 up{};

	// pSurface extent
	VkExtent2D extent{};

	float fov;

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

