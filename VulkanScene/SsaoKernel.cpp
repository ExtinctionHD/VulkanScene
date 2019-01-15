#include <ctime>
#include <vector>
#include <glm/glm.hpp>

#include "SsaoKernel.h"
#include <chrono>

// public:

SsaoKernel::SsaoKernel(Device *device)
{
	this->device = device;
	rndEngine = std::default_random_engine(unsigned(time(nullptr)));
	rndDist = std::uniform_real_distribution<float>(0.0f, 1.0f);

	createKernel();
	createNoiseTexture();
}

SsaoKernel::~SsaoKernel()
{
	delete kernelBuffer;
	delete noiseTexture;
}

Buffer* SsaoKernel::getKernelBuffer() const
{
	return kernelBuffer;
}

TextureImage * SsaoKernel::getNoiseTexture() const
{
	return noiseTexture;
}

// private:

void SsaoKernel::createKernel()
{
	std::vector<glm::vec4> kernel(SIZE);

	for (uint32_t i = 0; i < SIZE; ++i)
	{
		glm::vec3 sample(rndDist(rndEngine) * 2.0 - 1.0, rndDist(rndEngine) * 2.0 - 1.0, rndDist(rndEngine));
		sample = normalize(sample);
		sample *= rndDist(rndEngine);
		float scale = float(i) / float(SIZE);
		scale = lerp(0.1f, 1.0f, scale * scale);
		kernel[i] = glm::vec4(sample * scale, 0.0f);
	}

	kernelBuffer = new Buffer(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kernel.size() * VECTOR_SIZE);
	kernelBuffer->updateData(kernel.data(), kernel.size() * VECTOR_SIZE, 0);
}

void SsaoKernel::createNoiseTexture()
{
	std::vector<glm::vec4> noise(NOISE_DIM * NOISE_DIM);

	for (glm::vec4 &pixel : noise)
	{
		pixel = glm::vec4(rndDist(rndEngine) * 2.0f - 1.0f, rndDist(rndEngine) * 2.0f - 1.0f, 0.0f, 0.0f);
	}

    const VkExtent3D noiseExtent = { NOISE_DIM, NOISE_DIM, 1 };

	noiseTexture = new TextureImage(
		device,
		noiseExtent,
		0,
		VK_SAMPLE_COUNT_1_BIT,
		VK_FORMAT_R32G32B32A32_SFLOAT,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        1,
        false,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		VK_IMAGE_ASPECT_COLOR_BIT,
        VK_FILTER_LINEAR,
		VK_SAMPLER_ADDRESS_MODE_REPEAT);

	std::vector<glm::vec4*> data{ noise.data() };
	noiseTexture->updateData({ data.data() }, 0, VECTOR_SIZE);

	noiseTexture->transitLayout(
		device,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		{
			VK_IMAGE_ASPECT_COLOR_BIT,
			0,
			1,
			0,
			1
		});
}

float SsaoKernel::lerp(float a, float b, float f)
{
	return a + f * (b - a);
}
