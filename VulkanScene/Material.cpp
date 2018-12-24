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
	for (auto type : TEXTURES_ORDER)
	{
		textures.insert({ type, defaultTextures.at(type) });
	}

	colors = MaterialColors{
		glm::vec4(1.0f),	// diffuse
		glm::vec4(1.0f),	// specular
		1.0f				// opacity
	};

	pColorsBuffer = new Buffer(pDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(MaterialColors));
	updateColorsBuffer();

	objectCount++;
}

Material::~Material()
{
	objectCount--;

	delete(pColorsBuffer);

	if (objectCount == 0 && !defaultTextures.empty())
	{
		for (auto texture : defaultTextures)
		{
			delete texture.second;
		}
		defaultTextures.clear();
	}

	if (objectCount == 0 && dsLayout != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorSetLayout(pDevice->device, dsLayout, nullptr);
		dsLayout = VK_NULL_HANDLE;
	}
}

const std::vector<aiTextureType> Material::TEXTURES_ORDER = {
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

bool Material::isSolid() const
{
	return colors.opacity == 1.0f && textures.at(aiTextureType_OPACITY) == defaultTextures.at(aiTextureType_OPACITY);
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
    const std::string DEFAULT_TEXTURE_DIR = File::getExeDir() + "textures/default/";

	switch (type)
	{
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

void Material::initDescriptorSet(DescriptorPool * pDescriptorPool)
{
	std::vector<VkShaderStageFlags> texturesShaderStages(textures.size(), VK_SHADER_STAGE_FRAGMENT_BIT);

    if (dsLayout == VK_NULL_HANDLE)
    {
		dsLayout = pDescriptorPool->createDescriptorSetLayout({ VK_SHADER_STAGE_FRAGMENT_BIT }, texturesShaderStages);
    }

	descriptorSet = pDescriptorPool->getDescriptorSet(
		dsLayout
	);
	pDescriptorPool->updateDescriptorSet(
		descriptorSet,
		{ pColorsBuffer },
		getTextures()
	);
}

VkDescriptorSet Material::getDescriptorSet() const
{
	return descriptorSet;
}

VkDescriptorSetLayout Material::getDsLayout()
{
	return dsLayout;
}

void Material::initDefaultTextures(Device *pDevice)
{
	for (aiTextureType type : TEXTURES_ORDER)
	{
		defaultTextures.insert({ type, new TextureImage(pDevice, { getDefaultTexturePath(type) }, 1) });
	}
}

// private:

uint32_t Material::objectCount = 0;

VkDescriptorSetLayout Material::dsLayout = VK_NULL_HANDLE;

std::unordered_map<aiTextureType, TextureImage*> Material::defaultTextures;