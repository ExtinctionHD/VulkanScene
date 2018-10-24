#include "Vertex.h"
#include "Mesh.h"

#include "TerrainModel.h"

// public:

TerrainModel::TerrainModel(Device *pDevice, VkExtent2D size, VkExtent2D cellCount, std::string texturesDir, std::string extension) :
	Model(pDevice)
{
	this->size = size;
	this->cellCount = cellCount;

	initMaterial(texturesDir, extension);
	initMesh();
}


TerrainModel::~TerrainModel()
{
	for (TextureImage *pTexture : textures)
	{
		delete(pTexture);
	}
}

// protected:

VkVertexInputBindingDescription TerrainModel::getVertexInputBindingDescription(uint32_t inputBinding)
{
	return Vertex::getBindingDescription(inputBinding);
}

std::vector<VkVertexInputAttributeDescription> TerrainModel::getVertexInputAttributeDescriptions(uint32_t inputBinding)
{
	return Vertex::getAttributeDescriptions(inputBinding);
}

// private:

void TerrainModel::initMaterial(std::string texturesDir, std::string extension)
{
	const std::vector<aiTextureType> types = {
		aiTextureType_AMBIENT,
		aiTextureType_DIFFUSE,
		aiTextureType_SPECULAR,
		aiTextureType_NORMALS
	};

	const std::vector<std::string> filenames = {
		texturesDir + "ambient" + extension,
		texturesDir + "diffuse" + extension,
		texturesDir + "specular" + extension,
		texturesDir + "normals" + extension,
	};

	Material *pMaterial = new Material(pDevice);

	for (size_t i = 0; i < types.size(); i++)
	{
		if (File::exists(filenames[i]))
		{
			TextureImage *pTexture = new TextureImage(pDevice, { filenames[i] }, 1);
			pMaterial->addTexture(types[i], pTexture);
			textures.push_back(pTexture);
		}
	}

	materials.insert({ 0, pMaterial });
}

void TerrainModel::initMesh()
{
	const float posX = size.width / 2.0f;
	const float posZ = size.height / 2.0f;

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

	meshes.push_back(new Mesh<Vertex>(pDevice, vertices, indices, materials.at(0)));
}