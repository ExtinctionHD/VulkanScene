#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtc/matrix_transform.hpp>

#include "Lighting.h"

Lighting::Lighting(Device *device, Attributes attributes) : attributes(attributes)
{
	attributesBuffer = new Buffer(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof attributes);
	attributesBuffer->updateData(&attributes, sizeof attributes, 0);
}

Lighting::~Lighting()
{
	delete attributesBuffer;
}

glm::vec3 Lighting::getDirection() const
{
	return attributes.direction;
}

Buffer* Lighting::getAttributesBuffer() const
{
	return attributesBuffer;
}

void Lighting::update(glm::vec3 cameraPos)
{
	attributes.cameraPos = cameraPos;
	attributesBuffer->updateData(&attributes.cameraPos, sizeof attributes.cameraPos, offsetof(Attributes, cameraPos));
}
