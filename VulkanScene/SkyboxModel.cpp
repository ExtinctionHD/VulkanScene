#include "Position.h"
#include "File.h"

#include "SkyboxModel.h"

// public:

const std::vector<std::string> SkyboxModel::FILENAMES = {
	"right",
	"left",
	"top",
	"bottom",
	"front",
	"back",
};

SkyboxModel::SkyboxModel(Device *device, ImageSetInfo imageSetInfo) : Model(device, 1)
{
	std::vector<std::string> paths;
	for (auto &filename : FILENAMES)
	{
		paths.push_back(File::getPath(imageSetInfo.directory, filename + imageSetInfo.extension));
	}
	texture = new TextureImage(device, paths, CUBE_SIDE_COUNT, true, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT);

	const std::vector<Position> cubeVertices{
		glm::vec3(-1.0f, -1.0f, -1.0f),
		glm::vec3(-1.0f, 1.0f, -1.0f),
		glm::vec3(1.0f, -1.0f, -1.0f),
		glm::vec3(1.0f, 1.0f, -1.0f),
		glm::vec3(1.0f, -1.0f, 1.0f),
		glm::vec3(1.0f, 1.0f, 1.0f),
		glm::vec3(-1.0f, -1.0f, 1.0f),
		glm::vec3(-1.0f, 1.0f, 1.0f)
	};
	const std::vector<uint32_t> cubeIndices{
		0, 1, 2,
		2, 1, 3,
		2, 3, 4,
		4, 3, 5,
		4, 5, 6,
		6, 5, 7,
		6, 7, 0,
		0, 7, 1,
		6, 0, 2,
		2, 4, 6,
		7, 5, 3,
		7, 3, 1
	};

    auto material = new Material(device);
	materials.insert({ 0, material });
	material->addTexture(aiTextureType_DIFFUSE, texture);

	transparentMeshes.push_back(new Mesh<Position>(device, cubeVertices, cubeIndices, material));
}

SkyboxModel::~SkyboxModel()
{
	delete texture;
}

// protected:

VkVertexInputBindingDescription SkyboxModel::getVertexBindingDescription(uint32_t binding)
{
	return Position::getBindingDescription(binding);
}

std::vector<VkVertexInputAttributeDescription> SkyboxModel::getVertexAttributeDescriptions(
    uint32_t binding,
    uint32_t locationOffset)
{
	return Position::getAttributeDescriptions(binding, locationOffset);
}

// private:


