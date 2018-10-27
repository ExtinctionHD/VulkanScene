#include <glm/gtc/matrix_transform.hpp>

#include "Lighting.h"

Lighting::Lighting(Device *pDevice, Attributes attributes, float spaceRadius)
{
	this->attributes = attributes;

	pAttributesBuffer = new Buffer(pDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(attributes));
	pAttributesBuffer->updateData(&attributes, sizeof(attributes), 0);

	projection = glm::ortho(-spaceRadius, spaceRadius, -spaceRadius, spaceRadius, -spaceRadius, spaceRadius);
	projection[1][1] *= -1;

	glm::mat4 space = getSpaceMatrix();
	pSpaceBuffer = new Buffer(pDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_SHADER_STAGE_VERTEX_BIT, sizeof(glm::mat4) * 2);
	glm::mat4 view = lookAt(glm::vec3(0.0f) - normalize(attributes.direction), glm::vec3(0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
	pSpaceBuffer->updateData(&view, sizeof(view), 0);
	pSpaceBuffer->updateData(&projection, sizeof(projection), sizeof(view));
}

Lighting::~Lighting()
{
	delete pSpaceBuffer;
	delete pAttributesBuffer;
}

Buffer * Lighting::getAttributesBuffer() const
{
	return pAttributesBuffer;
}

Buffer* Lighting::getSpaceBuffer() const
{
	return pSpaceBuffer;
}

void Lighting::update(glm::vec3 cameraPos)
{
	attributes.cameraPos = cameraPos;
	pAttributesBuffer->updateData(&attributes.cameraPos, sizeof(attributes.cameraPos), offsetof(Attributes, cameraPos));

	glm::mat4 view = lookAt(glm::vec3(0.0f) - normalize(attributes.direction), glm::vec3(0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
	pSpaceBuffer->updateData(&view, sizeof(view), 0);
	pSpaceBuffer->updateData(&projection, sizeof(projection), sizeof(view));
}

glm::mat4 Lighting::getSpaceMatrix() const
{
	glm::mat4 view = lookAt(glm::vec3(0.0f) - normalize(attributes.direction), glm::vec3(0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
	return projection * view;
}
