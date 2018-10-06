#include "Material.h"
#include "File.h"
#include <iostream>

// public:

Material::Material(Device *pDevice)
{
	pColorsBuffer = new Buffer(pDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(MaterialColors));
}

Material::~Material()
{
	delete(pColorsBuffer);
}

const std::vector<aiTextureType> Material::TEXTURES_ORDER = {
	aiTextureType_AMBIENT,
	aiTextureType_DIFFUSE,
	aiTextureType_SPECULAR,
	aiTextureType_OPACITY,
	aiTextureType_NORMALS
};

std::vector<TextureImage*> Material::getTextures() const
{
	std::vector<TextureImage*> result;

	for (aiTextureType type : TEXTURES_ORDER)
	{
		try
		{
			result.push_back(textures.at(type));
		}
		catch (std::out_of_range ex) {}
	}

	return result;
}

void Material::updateColorsBuffer()
{
	pColorsBuffer->updateData(&colors, sizeof(MaterialColors), 0);
}

void Material::addTexture(aiTextureType type, TextureImage * pTexture)
{
	textures.insert(std::pair<aiTextureType, TextureImage*>(type, pTexture));
}

std::string Material::getDefaultTexturePath(aiTextureType type)
{
	std::string DEFAULT_TEXTURE_DIR = File::getExeDir() + "textures/default/";

	switch (type)
	{
	case aiTextureType_AMBIENT:
		return DEFAULT_TEXTURE_DIR + "ambient.jpg";
	case aiTextureType_DIFFUSE:
		return DEFAULT_TEXTURE_DIR + "diffuse.jpg";
	case aiTextureType_SPECULAR:
		return DEFAULT_TEXTURE_DIR + "specular.jpg";
	case aiTextureType_OPACITY:
		return DEFAULT_TEXTURE_DIR + "opacity.jpg";
	case aiTextureType_NORMALS:
		return DEFAULT_TEXTURE_DIR + "normals.jpg";
	default:
		throw std::invalid_argument("For this type no default texture");
	}
}
