#pragma once
#include "Image.h"

#include <stb_image.h>

// create texture image from file
class TextureImage : public Image
{
public:
	TextureImage(Device *pDevice, std::string filename);

	~TextureImage();

	// count of mipmap levels
	uint32_t mipLevels;

	// image in shader
	VkSampler sampler;

private:
	// returns pixel bytes and save image extent
	stbi_uc* loadPixels(std::string filename);

	// generate mipmap levels and transit image layout to SHADER_READ_ONLY
	void generateMipmaps(Device *pDevice, uint32_t arrayLayers = 1);

	void createSampler();
};

