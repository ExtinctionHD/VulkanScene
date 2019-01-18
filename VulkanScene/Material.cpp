#include "Material.h"
#include "File.h"
#include <iostream>

// public:

const std::vector<aiTextureType> Material::TEXTURES_ORDER{
	aiTextureType_DIFFUSE,
	aiTextureType_SPECULAR,
	aiTextureType_OPACITY,
	aiTextureType_NORMALS
};

const std::vector<RgbaUNorm> Material::DEFAULT_TEXTURES_COLORS{
	{ 255, 255, 255, 255 },
	{ 255, 255, 255, 255 },
	{ 255, 255, 255, 255 },
	{ 127, 127, 255, 255 },
};

Material::Material(Device *device) : device(device)
{
	if (defaultTextures.empty())
	{
		initDefaultTextures(device);
	}

	for (auto type : TEXTURES_ORDER)
	{
		textures.insert({ type, defaultTextures.at(type) });
	}

	colors = Colors{
		glm::vec4(1.0f),
		glm::vec4(1.0f),
		1.0f	
	};

	colorsBuffer = new Buffer(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(Colors));
	colorsBuffer->updateData(&colors, sizeof(Colors), 0);

	objectCount++;
}

Material::~Material()
{
	objectCount--;

	delete colorsBuffer;

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

std::vector<TextureImage*> Material::getTextures() const
{
	std::vector<TextureImage*> result;

    for (auto type : TEXTURES_ORDER)
	{
		result.push_back(textures.at(type));
	}

	return result;
}

void Material::setColors(Colors colors)
{
	this->colors = colors;

	colorsBuffer->updateData(&colors, sizeof(Colors), 0);
}

uint32_t Material::getIndex() const
{
	return index;
}

bool Material::solid() const
{
	return colors.opacity == 1.0f; // && textures.at(aiTextureType_OPACITY) == defaultTextures.at(aiTextureType_OPACITY);
}

void Material::addTexture(aiTextureType type, TextureImage *texture)
{
	textures.at(type) = texture;
}

void Material::initDescriptorSet(DescriptorPool *descriptorPool)
{
    const std::vector<VkShaderStageFlags> texturesShaderStages(textures.size(), VK_SHADER_STAGE_FRAGMENT_BIT);

    if (dsLayout == nullptr)
    {
		dsLayout = descriptorPool->createDescriptorSetLayout({ VK_SHADER_STAGE_FRAGMENT_BIT }, texturesShaderStages);
    }

	descriptorSet = descriptorPool->getDescriptorSet(dsLayout);
	descriptorPool->updateDescriptorSet(
		descriptorSet,
		{ colorsBuffer },
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

// private:

uint32_t Material::objectCount = 0;

VkDescriptorSetLayout Material::dsLayout = nullptr;

std::unordered_map<aiTextureType, TextureImage*> Material::defaultTextures;

void Material::initDefaultTextures(Device *device)
{
	for (uint32_t i = 0; i < TEXTURES_ORDER.size(); i++)
	{
		auto defaultTexture = new TextureImage(
			device,
			{ 1, 1, 1 },
			0,
			VK_SAMPLE_COUNT_1_BIT,
            1,
			VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT,
			1,
			false,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_IMAGE_ASPECT_COLOR_BIT,
			VK_FILTER_LINEAR,
			VK_SAMPLER_ADDRESS_MODE_REPEAT);

		const void *data = reinterpret_cast<const void*>(&DEFAULT_TEXTURES_COLORS[i]);
		defaultTexture->updateData({ data }, 0, sizeof RgbaUNorm);

		defaultTexture->transitLayout(
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