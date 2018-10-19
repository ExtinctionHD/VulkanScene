#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include "ViewProjMatrices.h"
#include "Buffer.h"

// provides camera attributes
// and movement functions
class Camera
{
public:
	Camera(Device *pDevice, VkExtent2D extent);

	Camera(Device *pDevice, glm::vec3 pos, glm::vec3 forward, glm::vec3 up, VkExtent2D extent);

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

	Buffer* getViewProjBuffer() const;

	glm::vec2 getCenter() const;

	void move(float deltaSec);

	void rotate(float deltaX, float deltaY);

	void setExtent(VkExtent2D extent);

	void updateView();

private:
	// step of camera movement
	const float SPEED = 15.0f;

	// camera rotation sensitivity
	const float SENSITIVITY = 0.1f;

	// position of camera
	glm::vec3 pos;

	// vector of camera direction
	glm::vec3 forward;

	// up vector of camera 
	glm::vec3 up;

	// pSurface extent
	VkExtent2D extent;

	// horizontal angle
	// angle 0.0f directed towards z axis
	float angleH;

	// vertical angle
	float angleV;

	// camera view and projection matrices
	ViewProjMatrices viewProj;
	Buffer *pViewProjBuffer;

	// initializes camera angles
	void initAngles();

	void initViewProj(Device *pDevice);

	glm::mat4 getViewMatrix() const;

	glm::mat4 getProjectionMatrix() const;
};

