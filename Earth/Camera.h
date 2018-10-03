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

	// returns camera position
	glm::vec3 getPos() const;

	// returns camera target position
	glm::vec3 getTarget() const;

	// returns camera up vector
	glm::vec3 getUp() const;

	// begins camera movement
	void onKeyDown(int key);

	// ends camera movement
	void onKeyUp(int key);

	void moveCamera(float deltaSec);

	// rotate camera on mouse movement
	void onMouseMove(float x, float y);

private:
	enum Direction
	{
		negative = -1,
		none = 0,
		positive = 1
	};

	// step of camera movement
	const float SPEED = 1.0f;

	// camera rotation sensitivity
	const float SENSITIVITY = 0.3f;

	// position of camera
	glm::vec3 pos;

	// vector of camera diraction
	glm::vec3 forward;

	// up vector of camera 
	glm::vec3 up;

	// surface extent
	VkExtent2D extent;

	// asix movement states
	struct Movement
	{
		Direction forward = Direction::none;
		Direction right = Direction::none;
		Direction up = Direction::none;
	} movement;

	// horizontal angle
	// angle 0.0f directed towards z axis
	float angleH;

	// vertical angle
	float angleV;

	// initializes camera angles
	void init();

	// returns x, y coordinates extent center
	glm::vec2 getCenter() const;
};

