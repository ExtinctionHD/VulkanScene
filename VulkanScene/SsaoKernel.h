#pragma once

#include "TextureImage.h"
#include "Buffer.h"
#include <random>

class SsaoKernel
{
public:
	SsaoKernel(Device *pDevice);
	~SsaoKernel();

	const uint32_t SIZE = 32;
	const uint32_t NOISE_DIM = 4;
	const float RADIUS = 0.4f;
	const float POWER = 1.4f;
	const uint32_t BLUR_RADIUS = 1;

	Buffer* getKernelBuffer() const;

	TextureImage* getNoiseTexture() const;

private:
	Device *pDevice;

    std::default_random_engine rndEngine;

	std::uniform_real_distribution<float> rndDist;

	Buffer *pKernelBuffer{};

	TextureImage *pNoiseTexture{};

	void createKernel();

	void createNoiseTexture();

    static float lerp(float a, float b, float f);
};

