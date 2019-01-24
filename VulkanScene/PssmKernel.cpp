#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtc/matrix_transform.hpp>

#include "PssmKernel.h"

// public:

PssmKernel::PssmKernel(Device *device, Camera *camera, glm::vec3 lightingDirection)
    : camera(camera), lightingDirection(lightingDirection)
{
	splitsBuffer = new Buffer(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, splitDepths.size() * sizeof(float));
	spacesBuffer = new Buffer(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, cascadeSpaces.size() * sizeof glm::mat4);

	update();
}

PssmKernel::~PssmKernel()
{
	delete splitsBuffer;
	delete spacesBuffer;
}

Buffer* PssmKernel::getSplitsBuffer() const
{
	return splitsBuffer;
}

Buffer* PssmKernel::getSpacesBuffer() const
{
	return spacesBuffer;
}

void PssmKernel::update()
{
	float nearPlane = camera->getNearPlane();
	float farPlane = camera->getFarPlane();
	float clipRange = farPlane - nearPlane;

	float minZ = nearPlane;
	float maxZ = nearPlane + clipRange;

	float range = maxZ - minZ;
	float ratio = maxZ / minZ;

	float cascadeSplits[CASCADE_COUNT];

	for (uint32_t i = 0; i < CASCADE_COUNT; i++)
	{
		float p = (i + 1) / float(CASCADE_COUNT);
		float log = minZ * std::pow(ratio, p);
		float uniform = minZ + range * p;
		float d = CASCADE_SPLIT_LAMBDA * (log - uniform) + uniform;
		cascadeSplits[i] = (d - nearPlane) / clipRange;
	}

	// Calculate orthographic projection matrix for each cascade
	float lastSplitDist = 0.0;
	for (uint32_t i = 0; i < CASCADE_COUNT; i++)
	{
		float splitDist = cascadeSplits[i];

		glm::vec3 frustumCorners[8] = {
			glm::vec3(-1.0f,  1.0f, -1.0f),
			glm::vec3(1.0f,  1.0f, -1.0f),
			glm::vec3(1.0f, -1.0f, -1.0f),
			glm::vec3(-1.0f, -1.0f, -1.0f),
			glm::vec3(-1.0f,  1.0f,  1.0f),
			glm::vec3(1.0f,  1.0f,  1.0f),
			glm::vec3(1.0f, -1.0f,  1.0f),
			glm::vec3(-1.0f, -1.0f,  1.0f),
		};

		// Project frustum corners into world space
		glm::mat4 invCam = inverse(camera->getProjectionMatrix() * camera->getViewMatrix());
		for (auto &frustumCorner : frustumCorners)
        {
			glm::vec4 invCorner = invCam * glm::vec4(frustumCorner, 1.0f);
            frustumCorner = invCorner / invCorner.w;
		}

		for (uint32_t j = 0; j < 4; j++)
		{
			glm::vec3 dist = frustumCorners[j + 4] - frustumCorners[j];
			frustumCorners[j + 4] = frustumCorners[j] + (dist * splitDist);
			frustumCorners[j] = frustumCorners[j] + (dist * lastSplitDist);
		}

		// Get frustum center
        auto frustumCenter = glm::vec3(0.0f);
		for (auto frustumCorner : frustumCorners)
        {
			frustumCenter += frustumCorner;
		}
		frustumCenter /= 8.0f;

		float radius = 0.0f;
		for (auto frustumCorner : frustumCorners)
        {
			float distance = glm::length(frustumCorner - frustumCenter);
			radius = glm::max(radius, distance);
		}
		radius = std::ceil(radius * 16.0f) / 16.0f;

        auto maxExtents = glm::vec3(radius);
		glm::vec3 minExtents = -maxExtents;

		glm::mat4 lightViewMatrix = glm::lookAt(
            frustumCenter - lightingDirection * -minExtents.z,
            frustumCenter,
            glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 lightOrthoMatrix = glm::ortho(
            minExtents.x,
            maxExtents.x,
            minExtents.y,
            maxExtents.y,
            0.0f,
            maxExtents.z - minExtents.z);

		// Store split distance and matrix in cascade
		splitDepths[i] = (nearPlane + splitDist * clipRange) * -1.0f;
		cascadeSpaces[i] = lightOrthoMatrix * lightViewMatrix;

		lastSplitDist = cascadeSplits[i];
	}

	splitsBuffer->updateData(splitDepths.data(), splitDepths.size(), 0);
	spacesBuffer->updateData(cascadeSpaces.data(), cascadeSpaces.size(), 0);
}
