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

	// moves camera on key pressing
	void onKeyPress(int key);

	// rotate camera on mouse movement
	void onMouseMove(float x, float y);

private:
	// step of camera movement
	const float STEP_SIZE = 0.1f;

	// camera rotation sensitivity
	const float SENSITIVITY = 0.005f;

	// position of camera
	glm::vec3 pos;

	// vector of camera diraction
	glm::vec3 forward;

	// up vector of camera 
	glm::vec3 up;

	// surface extent
	VkExtent2D extent;

	// horizontal angle
	float angleH;

	// vertical angle
	float angleV;

	void init();

	// returns x, y coordinates extent center
	glm::vec2 getCenter() const;
};

