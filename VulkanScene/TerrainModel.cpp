#include "Vertex.h"
#include "Mesh.h"
#include "File.h"

#include "TerrainModel.h"

// public:

TerrainModel::TerrainModel(
	Device *device, 
	glm::vec2 cellSize,
	VkExtent2D cellCount, 
	ImageSetInfo imageSetInfo) : Model(device, 1), cellSize(cellSize), cellCount(cellCount)
{
	initMaterial(imageSetInfo.directory, imageSetInfo.extension);
	initMesh();
}


TerrainModel::~TerrainModel()
{
	for (auto texture : textures)
	{
		delete texture;
	}
}

// protected:

VkVertexInputBindingDescription TerrainModel::getVertexBindingDescription(uint32_t binding)
{
	return Vertex::getBindingDescription(binding);
}

std::vector<VkVertexInputAttributeDescription> TerrainModel::getVertexAttributeDescriptions(
    uint32_t binding,
    uint32_t locationOffset)
{
	return Vertex::getAttributeDescriptions(binding, locationOffset);
}

// private:

void TerrainModel::initMaterial(const std::string &texturesDirectory, const std::string &extension)
{
	const std::vector<aiTextureType> types = {
		aiTextureType_DIFFUSE,
		aiTextureType_SPECULAR,
		aiTextureType_NORMALS
	};

	const std::vector<std::string> paths = {
		File::getPath(texturesDirectory, "diffuse" + extension),
		File::getPath(texturesDirectory, "specular" + extension),
		File::getPath(texturesDirectory, "normals" + extension),
	};

    auto material = new Material(device);

	for (size_t i = 0; i < types.size(); i++)
	{
		if (File::exists(paths[i]))
		{
			auto texture = new TextureImage(
                device,
                { paths[i] },
                1,
                false,
                VK_FILTER_LINEAR,
                VK_SAMPLER_ADDRESS_MODE_REPEAT);

			material->addTexture(types[i], texture);
			textures.push_back(texture);
		}
	}

	materials.insert({ 0, material });
}

void TerrainModel::initMesh()
{
	const float posX = cellSize.x * cellCount.width / 2.0f;
	const float posZ = cellSize.y * cellCount.height / 2.0f;

	std::vector<Vertex> vertices;

	// -x, -z
	Vertex vertex = {
		glm::vec3(-posX, 0.0f, -posZ),
		glm::vec2(0.0f, 0.0f),
		glm::vec3(0.0f, -1.0f, 0.0f),
		glm::vec3()
	};
	vertices.push_back(vertex);

	// x, -z
	vertex = {
		glm::vec3(posX, 0.0f, -posZ),
		glm::vec2(cellCount.width, 0.0f),
		glm::vec3(0.0f, -1.0f, 0.0f),
		glm::vec3()
	};
	vertices.push_back(vertex);

	// x, z
	vertex = {
		glm::vec3(posX, 0.0f, posZ),
		glm::vec2(cellCount.width, cellCount.height),
		glm::vec3(0.0f, -1.0f, 0.0f),
		glm::vec3()
	};
	vertices.push_back(vertex);

	// -x, z
	vertex = {
		glm::vec3(-posX, 0.0f, posZ),
		glm::vec2(0.0f, cellCount.height),
		glm::vec3(0.0f, -1.0f, 0.0f),
		glm::vec3(1.0f, 0.0f, 0.0f)
	};
	vertices.push_back(vertex);

	const std::vector<uint32_t> indices{
		0, 1, 3,
		1, 2, 3
	};

	solidMeshes.push_back(new Mesh<Vertex>(device, vertices, indices, materials.at(0)));
}