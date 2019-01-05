#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

class Transformation
{
public:
	Transformation() = default;

	Transformation(glm::mat4 matrix);

	glm::mat4 getMatrix() const;

	void setMatrix(glm::mat4 matrix);

	void rotate(glm::vec3 axis, float angle);

	void rotateAxisX(float angle);

	void rotateAxisY(float angle);

	void rotateAxisZ(float angle);

	void move(glm::vec3 distance);

	void scale(glm::vec3 scale);

private:
	glm::mat4 matrix = glm::mat4(1.0f);
};

