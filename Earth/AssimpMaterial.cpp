#include "AssimpMaterial.h"
#include "File.h"
#include <iostream>

// public:

AssimpMaterial::AssimpMaterial(Device *pDevice)
{
	pColorsBuffer = new Buffer(pDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(MaterialColors));
}

AssimpMaterial::~AssimpMaterial()
{
	delete(pColorsBuffer);
}

const std::vector<aiTextureType> AssimpMaterial::TEXTURES_ORDER = {
	aiTextureType_DIFFUSE,
	aiTextureType_SPECULAR,
	aiTextureType_NORMALS,
	aiTextureType_OPACITY
};

std::vector<TextureImage*> AssimpMaterial::getTextures() const
{
	std::vector<TextureImage*> result;

	for (aiTextureType type : TEXTURES_ORDER)
	{
		result.push_back(textures.at(type));
	}

	return result;
}

void AssimpMaterial::updateColorsBuffer()
{
	pColorsBuffer->updateData(&colors, sizeof(MaterialColors), 0);
}

void AssimpMaterial::addTexture(aiTextureType type, TextureImage * pTexture)
{
	textures.insert(std::pair<aiTextureType, TextureImage*>(type, pTexture));
}

std::string AssimpMaterial::getDefaultTexturePath(aiTextureType type)
{
	std::string DEFAULT_TEXTURE_DIR = File::getExeDir() + "textures/default/";

	switch (type)
	{
	case aiTextureType_DIFFUSE:
		return DEFAULT_TEXTURE_DIR + "texture.jpg";
	case aiTextureType_SPECULAR:
		return DEFAULT_TEXTURE_DIR + "texture.jpg";
	case aiTextureType_NORMALS:
		return DEFAULT_TEXTURE_DIR + "texture.jpg";
	case aiTextureType_OPACITY:
		return DEFAULT_TEXTURE_DIR + "texture.jpg";
	default:
		throw std::invalid_argument("For this type no default texture");
	}
}
