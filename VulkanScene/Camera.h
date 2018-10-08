#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

// provides camera attributes
// and movement functions
class Camera
{
public:
	Camera(VkExtent2D extent);

	Camera(glm::vec3 pos, glm::vec3 forward, glm::vec3 up, VkExtent2D extent);

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

	void moveCamera(float deltaSec);

	void rotateCamera(float deltaX, float deltaY);

	void setCameraExtent(VkExtent2D extent);

	glm::mat4 getViewMatrix() const;

	glm::mat4 getProjectionMatrix() const;

	glm::vec2 getCenter() const;

private:

	// step of camera movement
	const float SPEED = 15.0f;

	// camera rotation sensitivity
	const float SENSITIVITY = 0.2f;

	// position of camera
	glm::vec3 pos;

	// vector of camera diraction
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

	// initializes camera angles
	void init();
};

