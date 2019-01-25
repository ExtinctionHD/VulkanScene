#pragma once

#include <glm/glm.hpp>
#include <vector>
#include "Buffer.h"
#include "Camera.h"

class PssmKernel
{
public:
	static const uint32_t CASCADE_COUNT = 4; 
    
    const float CASCADE_SPLIT_LAMBDA = 0.9f;

	PssmKernel(Device *device, Camera *camera, glm::vec3 lightingDirection);
	~PssmKernel();

	Buffer* getSplitsBuffer() const;

	Buffer* getSpacesBuffer() const;

	void update();

private:
	std::vector<float> cascadeSplits;

	std::vector<glm::mat4> cascadeSpaces;

	Camera *camera;

	glm::vec3 lightingDirection;

	Buffer *splitsBuffer;

	Buffer *spacesBuffer;
};

