#pragma once
#include "Image.h"

#include <stb_image.h>
#include <string>

// create texture image from file
class TextureImage : public Image
{
public:
	TextureImage() = default;

	TextureImage(
		Device *device,
		const std::vector<std::string> &paths,
		uint32_t arrayLayers,
		bool cubeMap,
		VkFilter filter,
		VkSamplerAddressMode samplerAddressMode);

	TextureImage(
		Device *device,
		VkExtent3D extent,
		VkImageCreateFlags flags,
		VkSampleCountFlagBits sampleCount,
		uint32_t mipLevels,
		VkFormat format,
		VkImageTiling tiling,
		VkImageUsageFlags usage,
		uint32_t arrayLayers,
		bool cubeMap,
		VkMemoryPropertyFlags properties,
		VkImageAspectFlags aspectFlags,
		VkFilter filter,
		VkSamplerAddressMode samplerAddressMode);

	~TextureImage();

	VkSampler getSampler() const;

protected:
	VkSampler sampler;

	stbi_uc* loadPixels(const std::string &path);

	// generate mipmap levels and transit image layout to SHADER_READ_ONLY
	void generateMipmaps(Device *device, VkImageAspectFlags aspectFlags, VkFilter filter) const;

	void createSampler(VkFilter filter, VkSamplerAddressMode addressMode);
};

