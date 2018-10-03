#pragma once
#include "Image.h"

#include <stb_image.h>

// create texture image from file
class TextureImage : 
	public Image
{
public:
	TextureImage() {}

	// array layer count must be equal to filenames count
	// images must have same extent
	TextureImage(Device *pDevice, std::vector<std::string> filenames, uint32_t arrayLayers, bool isCube = false);

	~TextureImage();

	// count of mipmap levels
	uint32_t mipLevels;

	// image in shader
	VkSampler sampler;

protected:
	// returns pixel bytes and save image extent
	stbi_uc* loadPixels(std::string filename);

	// generate mipmap levels and transit image layout to SHADER_READ_ONLY
	void generateMipmaps(Device *pDevice, uint32_t arrayLayers);

	void createSampler();
};

