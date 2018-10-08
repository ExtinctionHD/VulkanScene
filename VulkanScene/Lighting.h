#pragma once

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