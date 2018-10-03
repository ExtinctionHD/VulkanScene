#include "SkyboxModel.h"

// public:

SkyboxModel::SkyboxModel(Device *pDevice, std::string texturesDir, std::string extension) :
	Model(pDevice)
{
	std::vector<std::string> filenames = {
		texturesDir + "right" + extension,
		texturesDir + "left" + extension,
		texturesDir + "top" + extension,
		texturesDir + "bottom" + extension,
		texturesDir + "front" + extension,
		texturesDir + "back" + extension,
	};

	pTexture = new TextureImage(pDevice, filenames, CUBE_SIDE_COUNT, true);

	std::vector<Position> cubeVertices{
		glm::vec3(-1.0f, -1.0f, -1.0f),
		glm::vec3(-1.0f, 1.0f, -1.0f),
		glm::vec3(1.0f, -1.0f, -1.0f),
		glm::vec3(1.0f, 1.0f, -1.0f),
		glm::vec3(1.0f, -1.0f, 1.0f),
		glm::vec3(1.0f, 1.0f, 1.0f),
		glm::vec3(-1.0f, -1.0f, 1.0f),
		glm::vec3(-1.0f, 1.0f, 1.0f)
	};
	std::vector<uint32_t> cubeIndices{
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
	pMesh = new Mesh<Position>(pDevice, cubeVertices, cubeIndices, nullptr);

	objectCount++;
}

SkyboxModel::~SkyboxModel()
{
	objectCount--;

	if (objectCount == 0 && meshDSLayout != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorSetLayout(pDevice->device, meshDSLayout, nullptr);
		meshDSLayout = VK_NULL_HANDLE;
	}

	delete(pMesh);
	delete(pTexture);
}

// protected:

void SkyboxModel::initMeshDescriptorSets(DescriptorPool * pDescriptorPool)
{
	meshDescriptorSets.push_back(
		pDescriptorPool->getDescriptorSet(
			{ pMesh->getMaterialColorBuffer() }, 
			pMesh->getMaterialTextures(), 
			meshDSLayout == VK_NULL_HANDLE, 
			meshDSLayout
		)
	);
}

// private:

uint32_t SkyboxModel::objectCount = 0;

VkDescriptorSetLayout SkyboxModel::meshDSLayout = VK_NULL_HANDLE;

