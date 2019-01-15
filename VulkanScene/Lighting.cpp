#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtc/matrix_transform.hpp>

#include "Lighting.h"

Lighting::Lighting(Device *device, Attributes attributes, float spaceRadius) : attributes(attributes)
{
	attributesBuffer = new Buffer(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof attributes);
	attributesBuffer->updateData(&attributes, sizeof attributes, 0);

	projection = glm::ortho(-spaceRadius, spaceRadius, -spaceRadius, spaceRadius, -spaceRadius, spaceRadius);
	projection[1][1] *= -1;

	Space space = getSpace();
	spaceBuffer = new Buffer(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof space);
	spaceBuffer->updateData(&space, sizeof space, 0);
}

Lighting::~Lighting()
{
	delete spaceBuffer;
	delete attributesBuffer;
}

Buffer* Lighting::getAttributesBuffer() const
{
	return attributesBuffer;
}

Buffer* Lighting::getSpaceBuffer() const
{
	return spaceBuffer;
}

void Lighting::update(glm::vec3 cameraPos)
{
	attributes.cameraPos = cameraPos;
	attributesBuffer->updateData(&attributes.cameraPos, sizeof attributes.cameraPos, offsetof(Attributes, cameraPos));

	Space space = getSpace();
	spaceBuffer->updateData(&space, sizeof space, 0);
}

Space Lighting::getSpace() const
{
	return Space{
		   lookAt(attributes.cameraPos - normalize(attributes.direction), attributes.cameraPos, glm::vec3(0.0f, -1.0f, 0.0f)),
		   projection
	};
}
