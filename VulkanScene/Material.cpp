#include "Material.h"
#include "File.h"
#include <iostream>

// public:

Material::Material(Device *device)
{
	this->device = device;

	// create static vector with default textures
	if (defaultTextures.empty())
	{
		initDefaultTextures(device);
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

	pColorsBuffer = new Buffer(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(MaterialColors));
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

	if (objectCount == 0 && dsLayout != nullptr)
	{
		vkDestroyDescriptorSetLayout(device->get(), dsLayout, nullptr);
		dsLayout = nullptr;
	}
}

const std::vector<aiTextureType> Material::TEXTURES_ORDER = {
	aiTextureType_DIFFUSE,
	aiTextureType_SPECULAR,
	aiTextureType_OPACITY,
	aiTextureType_NORMALS
};

std::vector<RgbaUNorm> Material::DEFAULT_TEXTURES_COLORS = {
	{ 255, 255, 255, 255 },
	{ 255, 255, 255, 255 },
	{ 255, 255, 255, 255 },
	{ 127, 127, 255, 255 },
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
	return colors.opacity == 1.0f; // && textures.at(aiTextureType_OPACITY) == defaultTextures.at(aiTextureType_OPACITY);
}

void Material::updateColorsBuffer()
{
	pColorsBuffer->updateData(&colors, sizeof(MaterialColors), 0);
}

void Material::addTexture(aiTextureType type, TextureImage * pTexture)
{
	textures.at(type) = pTexture;
}

void Material::initDescriptorSet(DescriptorPool * pDescriptorPool)
{
	std::vector<VkShaderStageFlags> texturesShaderStages(textures.size(), VK_SHADER_STAGE_FRAGMENT_BIT);

    if (dsLayout == nullptr)
    {
		dsLayout = pDescriptorPool->createDescriptorSetLayout({ VK_SHADER_STAGE_FRAGMENT_BIT }, texturesShaderStages);
    }

	descriptorSet = pDescriptorPool->getDescriptorSet(
		dsLayout);
	pDescriptorPool->updateDescriptorSet(
		descriptorSet,
		{ pColorsBuffer },
		getTextures());
}

VkDescriptorSet Material::getDescriptorSet() const
{
	return descriptorSet;
}

VkDescriptorSetLayout Material::getDsLayout()
{
	return dsLayout;
}

void Material::initDefaultTextures(Device *device)
{
	for (uint32_t i = 0; i < TEXTURES_ORDER.size(); i++)
	{
		auto defaultTexture = new TextureImage(
			device,
			{ 1, 1, 1 },
			0,
			VK_SAMPLE_COUNT_1_BIT,
			VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT,
			1,
			false,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_IMAGE_ASPECT_COLOR_BIT,
            VK_FILTER_LINEAR,
			VK_SAMPLER_ADDRESS_MODE_REPEAT);

		void *data = reinterpret_cast<void*>(&DEFAULT_TEXTURES_COLORS[i]);
		defaultTexture->updateData({ data }, 0, sizeof RgbaUNorm);

		defaultTexture->transitLayout(
			device,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			{
				VK_IMAGE_ASPECT_COLOR_BIT,
				0,
				1,
				0,
				1
			});

		defaultTextures.insert({ TEXTURES_ORDER[i], defaultTexture });
	}
}

// private:

uint32_t Material::objectCount = 0;

VkDescriptorSetLayout Material::dsLayout = nullptr;

std::unordered_map<aiTextureType, TextureImage*> Material::defaultTextures;