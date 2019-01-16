#pragma once

#include "TextureImage.h"
#include "Buffer.h"
#include <random>

class SsaoKernel
{
public:
	SsaoKernel(Device *device);
	~SsaoKernel();

	const uint32_t VECTOR_SIZE = sizeof glm::vec4;

	const uint32_t SIZE = 32;
	const uint32_t NOISE_DIM = 4;
	const float RADIUS = 0.4f;
	const float POWER = 1.4f;

	const uint32_t BLUR_RADIUS = 2;

	Buffer* getKernelBuffer() const;

	TextureImage* getNoiseTexture() const;

private:
	Device *device;

    std::default_random_engine rndEngine;

	std::uniform_real_distribution<float> rndDist;

	Buffer *kernelBuffer;

	TextureImage *noiseTexture;

	void createKernel();

	void createNoiseTexture();

    static float lerp(float a, float b, float f);
};

