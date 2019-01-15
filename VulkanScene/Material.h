#pragma once

#include <vector>
#include "TextureImage.h"
#include <glm/glm.hpp>
#include <assimp/scene.h>
#include <unordered_map>
#include "Buffer.h"
#include "DescriptorPool.h"
#include "RgbaUNorm.h"

class Material
{
public:
    struct Colors
    {
	    glm::vec4 diffuseColor;

	    glm::vec4 specularColor;

	    float opacity;
	};

	static const std::vector<aiTextureType> TEXTURES_ORDER;

	Material(Device *device);

	~Material();

	std::vector<TextureImage*> getTextures() const;

	void setColors(Colors colors);

	uint32_t getIndex() const;

	bool solid() const;

	void addTexture(aiTextureType type, TextureImage *texture);

	void initDescriptorSet(DescriptorPool *descriptorPool);

	VkDescriptorSet getDescriptorSet() const;

	static VkDescriptorSetLayout getDsLayout();

private:
	static std::vector<RgbaUNorm> DEFAULT_TEXTURES_COLORS;

	Device *device;

	Colors colors;

	uint32_t index;

	Buffer *colorsBuffer;

	std::unordered_map<aiTextureType, TextureImage*> textures;

	VkDescriptorSet descriptorSet;

	static uint32_t objectCount;

	static VkDescriptorSetLayout dsLayout;

	static std::unordered_map<aiTextureType, TextureImage*> defaultTextures;

	static void initDefaultTextures(Device *device);
};

