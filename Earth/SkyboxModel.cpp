#include "SkyboxModel.h"

// public:

SkyboxModel::SkyboxModel(Device *pDevice, std::array<std::string, CUBE_SIDE_COUNT> textureFilenames) :
	Model(pDevice)
{
	std::vector<std::string> filenames = std::vector<std::string>(textureFilenames.begin(), textureFilenames.end());
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
	pMesh = new AssimpMesh<Position>(pDevice, cubeVertices, cubeIndices, nullptr);
}

SkyboxModel::~SkyboxModel()
{
	delete(pMesh);
	delete(pTexture);
}
