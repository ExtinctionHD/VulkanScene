#include "SkyboxModel.h"

// public:

SkyboxModel::SkyboxModel(Device *pDevice, std::string texturesDir, std::string extension) : 
	Model(pDevice)
{
	const std::vector<std::string> filenames = {
		texturesDir + "right" + extension,
		texturesDir + "left" + extension,
		texturesDir + "top" + extension,
		texturesDir + "bottom" + extension,
		texturesDir + "front" + extension,
		texturesDir + "back" + extension,
	};
	pTexture = new TextureImage(pDevice, filenames, CUBE_SIDE_COUNT, true);

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

	Material *pMaterial = new Material(pDevice);
	materials.insert(std::pair<uint32_t, Material*>(0, pMaterial));
	pMaterial->addTexture(aiTextureType_AMBIENT, pTexture);

	meshes.push_back(new Mesh<Position>(pDevice, cubeVertices, cubeIndices, pMaterial));
}

SkyboxModel::~SkyboxModel()
{
	delete(pTexture);
}

// protected:

VkVertexInputBindingDescription SkyboxModel::getVertexInputBindingDescription(uint32_t inputBinding)
{
	return Position::getBindingDescription(inputBinding);
}

std::vector<VkVertexInputAttributeDescription> SkyboxModel::getVertexInputAttributeDescriptions(uint32_t inputBinding)
{
	return Position::getAttributeDescriptions(inputBinding);
}

// private:


