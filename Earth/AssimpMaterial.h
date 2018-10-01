#pragma once

#include <vector>
#include "TextureImage.h"
#include <glm/glm.hpp>
#include <string>
#include <assimp/scene.h>

class AssimpMaterial
{
public:
	struct MaterialColors
	{
		glm::vec4 ambientColor;

		glm::vec4 diffuseColor;

		glm::vec4 specularColor;

		float opacity;
	};

	uint32_t index;

	TextureImage *pTexture;

	TextureImage *pOpacityMap;

	TextureImage *pSpecularMap;

	TextureImage *pNormalMap;

	MaterialColors colors;

	static std::string getDefaultTexturePath(aiTextureType type);
};

