#pragma once

#include <glm/glm.hpp>
#include "Buffer.h"

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

	Lighting(Device *pDevice, Attributes attributes, float spaceRadius);
	~Lighting();

	Buffer* getAttributesBuffer() const;

    Buffer* getSpaceBuffer() const;

	void update(glm::vec3 cameraPos);

private:
	Attributes attributes;

	glm::mat4 projection;

	Buffer *pAttributesBuffer;

	Buffer *pSpaceBuffer;

	glm::mat4 getSpaceMatrix() const;
};
