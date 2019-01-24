#pragma once

#include "TextureImage.h"
#include "Buffer.h"
#include <random>

class SsaoKernel
{
public:
	const uint32_t SIZE = 32;
	const uint32_t NOISE_DIM = 4;
	const float RADIUS = 0.4f;
	const float POWER = 1.0f;

	const uint32_t BLUR_RADIUS = 2;

	SsaoKernel(Device *device);
	~SsaoKernel();

	Buffer* getBuffer() const;

	TextureImage* getNoiseTexture() const;

private:
	Device *device;

    std::default_random_engine rndEngine;

	std::uniform_real_distribution<float> rndDist;

	Buffer *kernelBuffer;

	TextureImage *noiseTexture;

	void createKernelBuffer();

	void createNoiseTexture();

    static float lerp(float a, float b, float f);
};

