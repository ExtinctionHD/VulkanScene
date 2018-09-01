#pragma once

#include <glm/glm.hpp>

struct Lighting
{
	glm::vec3 direction;

	float ambientStrength;

	glm::vec3 color;
};