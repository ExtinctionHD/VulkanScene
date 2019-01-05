#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

#include "Transformation.h"

Transformation::Transformation(glm::mat4 matrix)
{
	this->matrix = matrix;
}

glm::mat4 Transformation::getMatrix() const
{
	return matrix;
}

void Transformation::setMatrix(glm::mat4 matrix)
{
	this->matrix = matrix;
}
void Transformation::rotate(glm::vec3 axis, float angle)
{
	matrix = glm::rotate(matrix, glm::radians(angle), axis);
}

void Transformation::rotateAxisX(float angle)
{
	matrix = glm::rotate(matrix, glm::radians(angle), glm::vec3(1.0f, 0.0f, 0.0f));
}

void Transformation::rotateAxisY(float angle)
{
	matrix = glm::rotate(matrix, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
}

void Transformation::rotateAxisZ(float angle)
{
	matrix = glm::rotate(matrix, glm::radians(angle), glm::vec3(0.0f,0.0f, 1.0f));
}

void Transformation::move(glm::vec3 distance)
{
	matrix = translate(matrix, distance);
}

void Transformation::scale(glm::vec3 scale)
{
	matrix = glm::scale(matrix, scale);
}
