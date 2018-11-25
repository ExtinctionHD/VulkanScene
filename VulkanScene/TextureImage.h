#pragma once
#include "Image.h"

#include <stb_image.h>

// create texture image from file
class TextureImage : 
	public Image
{
public:
	TextureImage() = default;

    // loads texture from files
	// array layer count must be equal to filenames count
	// images must have same extent
	TextureImage(Device *pDevice, std::vector<std::string> filenames, uint32_t arrayLayers, bool isCube = false);

    // creates device local texture
	TextureImage(
	    Device *pDevice,
	    VkExtent3D extent,
		VkImageCreateFlags flags,
		VkSampleCountFlagBits sampleCount,
	    VkFormat format,
		VkImageTiling tiling,
		VkImageUsageFlags usage,
		VkMemoryPropertyFlags properties,
	    VkImageAspectFlags aspectFlags,
        VkImageViewType viewType,
	    uint32_t arrayLayers
	);

	~TextureImage();

	// count of mipmap levels
	uint32_t mipLevels{};

	// image in shader
	VkSampler sampler{};

protected:
	// returns pixel bytes and save image extent
	stbi_uc* loadPixels(const std::string &filename);

	// generate mipmap levels and transit image layout to SHADER_READ_ONLY
	void generateMipmaps(Device *pDevice, uint32_t arrayLayers, VkImageAspectFlags aspectFlags);

	void createSampler(VkSamplerAddressMode addressMode);
};

