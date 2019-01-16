#pragma once

#include <glm/glm.hpp>
#include "Buffer.h"
#include "Space.h"

class Lighting
{
public:
	struct Attributes
	{
		glm::vec3 color;

		float ambientStrength;

		glm::vec3 direction;

		float directedStrength;

		glm::vec3 cameraPos;

		float specularPower;
	};

	Lighting(Device *device, Attributes attributes, float spaceRadius);
	~Lighting();

	Buffer* getAttributesBuffer() const;

    Buffer* getSpaceBuffer() const;

	void update(glm::vec3 cameraPos);

private:
	Attributes attributes;

	glm::mat4 projection;

	Buffer *attributesBuffer;

	Buffer *spaceBuffer;

	Space getSpace() const;
};
