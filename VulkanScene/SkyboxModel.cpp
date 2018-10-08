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

	pMaterial = new Material(pDevice);
	pMaterial->addTexture(aiTextureType_AMBIENT, pTexture);

	meshes.push_back(new Mesh<Position>(pDevice, cubeVertices, cubeIndices, pMaterial));

	objectCount++;
}

SkyboxModel::~SkyboxModel()
{
	objectCount--;

	if (objectCount == 0)
	{
		if (meshDSLayout != VK_NULL_HANDLE)
		{
			vkDestroyDescriptorSetLayout(pDevice->device, meshDSLayout, nullptr);
			meshDSLayout = VK_NULL_HANDLE;
		}
	}

	delete(pMaterial);
	delete(pTexture);
}

void SkyboxModel::createPipeline(Device * pDevice, std::vector<VkDescriptorSetLayout> layouts, RenderPass * pRenderPass)
{
	layouts.push_back(mvpDSLayout);
	layouts.push_back(meshDSLayout);

	if (pPipeline == nullptr)
	{
		delete(pPipeline);
	}

	const uint32_t inputBinding = 0;

	pPipeline = new GraphicsPipeline(
		pDevice->device,
		layouts,
		pRenderPass,
		{
			new ShaderModule(pDevice->device, SHADER_FILES[ShaderTypes::vert], VK_SHADER_STAGE_VERTEX_BIT),
			new ShaderModule(pDevice->device, SHADER_FILES[ShaderTypes::frag], VK_SHADER_STAGE_FRAGMENT_BIT)
		},
		Position::getBindingDescription(inputBinding),
		Position::getAttributeDescriptions(inputBinding)
	);
}

void SkyboxModel::recreatePipeline(RenderPass * pRenderPass)
{
	if (pPipeline == nullptr)
	{
		throw std::runtime_error("Pipeline not created yet");
	}

	pPipeline->recreate(pRenderPass);
}

void SkyboxModel::destroyPipeline()
{
	if (pPipeline != nullptr)
	{
		delete(pPipeline);
		pPipeline = nullptr;
	}
}

// protected:

VkDescriptorSetLayout& SkyboxModel::getMeshDSLayout()
{
	return meshDSLayout;
}

GraphicsPipeline * SkyboxModel::getPipeline()
{
	return pPipeline;
}

// private:

const std::vector<std::string> SkyboxModel::SHADER_FILES = {
	File::getExeDir() + "shaders/skybox/vert.spv",
	File::getExeDir() + "shaders/skybox/frag.spv"
};

uint32_t SkyboxModel::objectCount = 0;

VkDescriptorSetLayout SkyboxModel::meshDSLayout = VK_NULL_HANDLE;

GraphicsPipeline* SkyboxModel::pPipeline = nullptr;

