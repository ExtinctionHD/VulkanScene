#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

struct Lighting
{
	glm::vec3 color;

	float ambientStrength;

	glm::vec3 direction;

	float diffuesStrength;

	glm::vec3 cameraPos;

	float specularPower;
};