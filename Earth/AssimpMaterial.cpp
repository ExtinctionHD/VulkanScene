#include "AssimpMaterial.h"
#include "File.h"
#include <iostream>

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
