#include "Material.h"
#include "File.h"
#include <iostream>

// public:

Material::Material(Device *pDevice)
{
	this->pDevice = pDevice;

	// create static vector with default textures
	if (defaultTextures.empty())
	{
		initDefaultTextures(pDevice);
	}

	// initialize current material with default textures
	for (int i = 0; i < TEXTURES_ORDER.size(); i++)
	{
		textures.insert(std::pair<aiTextureType, TextureImage*>(TEXTURES_ORDER[i], defaultTextures[i]));
	}

	colors = MaterialColors{
		glm::vec4(1.0f),	// ambient
		glm::vec4(1.0f),	// diffuse
		glm::vec4(1.0f),	// specular
		1.0f				// opacity
	};

	pColorsBuffer = new Buffer(pDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(MaterialColors));
	updateColorsBuffer();

	objectCount++;
}

Material::~Material()
{
	objectCount--;

	delete(pColorsBuffer);

	if (objectCount == 0 && dsLayout != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorSetLayout(pDevice->device, dsLayout, nullptr);
		dsLayout = VK_NULL_HANDLE;
	}
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
		result.push_back(textures.at(type));
	}

	return result;
}

void Material::updateColorsBuffer()
{
	pColorsBuffer->updateData(&colors, sizeof(MaterialColors), 0);
}

void Material::addTexture(aiTextureType type, TextureImage * pTexture)
{
	textures.at(type) = pTexture;
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

void Material::initDescritorSet(DescriptorPool * pDescriptorPool)
{
	descriptorSet = pDescriptorPool->getDescriptorSet({ pColorsBuffer }, getTextures(), dsLayout == VK_NULL_HANDLE, dsLayout);
}

VkDescriptorSet Material::getDesriptorSet() const
{
	return descriptorSet;
}

VkDescriptorSetLayout Material::getDSLayout()
{
	return dsLayout;
}

void Material::initDefaultTextures(Device *pDevice)
{
	for (aiTextureType type : TEXTURES_ORDER)
	{
		defaultTextures.push_back(new TextureImage(pDevice, { getDefaultTexturePath(type) }, 1));
	}
}

// private:

uint32_t Material::objectCount = 0;

VkDescriptorSetLayout Material::dsLayout = VK_NULL_HANDLE;

std::vector<TextureImage*> Material::defaultTextures;