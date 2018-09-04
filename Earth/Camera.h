#pragma once

#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

// provides camera attributes
// and movement functions
class Camera
{
public:
	Camera();

	Camera(glm::vec3 pos, glm::vec3 forward, glm::vec3 up);

	~Camera();

	// returns camera position
	glm::vec3 getPos() const;

	// returns camera target position
	glm::vec3 getTarget() const;

	// returns camera up vector
	glm::vec3 getUp() const;

	// moves camera on key pressing
	bool onKeyPress(int key);

private:
	// step of camera movement
	const float STEP_SIZE = 0.1f;

	// position of camera
	glm::vec3 pos;

	// vector of camera diraction
	glm::vec3 forward;

	// up vector of camera 
	glm::vec3 up;
};

