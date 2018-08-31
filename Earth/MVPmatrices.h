#pragma once

#include <glm/glm.hpp>

struct MvpMatrices
{
	glm::mat4 model = glm::mat4(1);
	glm::mat4 view;
	glm::mat4 proj;
};