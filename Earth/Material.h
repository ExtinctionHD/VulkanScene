#pragma once

#include <vector>
#include "TextureImage.h"
#include <glm/glm.hpp>
#include <string>
#include <assimp/scene.h>
#include <unordered_map>
#include "Buffer.h"

class Material
{
public:
	Material(Device *pDevice);

	~Material();

	// order of textures for each material
	static const std::vector<aiTextureType> TEXTURES_ORDER;

	uint32_t index;

	struct MaterialColors
	{
		glm::vec4 ambientColor;

		glm::vec4 diffuseColor;

		glm::vec4 specularColor;

		float opacity;
	} colors;

	std::vector<TextureImage*> getTextures() const;

	Buffer *pColorsBuffer;

	// loads colors data in buffer
	void updateColorsBuffer();

	// adds texture of this type
	void addTexture(aiTextureType type, TextureImage *pTexture);


	static std::string getDefaultTexturePath(aiTextureType type);

private:
	std::unordered_map<aiTextureType, TextureImage*> textures;
};

