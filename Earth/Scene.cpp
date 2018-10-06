#include "AssimpModel.h"
#include "SkyboxModel.h"
#include <glm/gtc/matrix_transform.hpp>

#include "Scene.h"

// public:

Scene::Scene(Device *pDevice, VkExtent2D cameraExtent)
{
	this->pDevice = pDevice;

	initCamera(cameraExtent);
	initLighting();
	initModels();

	pController = new Controller(pCamera);
}

Scene::~Scene()
{
	AssimpModel::destroyPipeline();
	SkyboxModel::destroyPipeline();

	for (Model *pModel : models)
	{
		delete(pModel);
	}

	vkDestroyDescriptorSetLayout(pDevice->device, lightingDSLayout, nullptr);

	delete(pLightingBuffer);
	delete(pCamera);
	delete(pController);
}

Controller* Scene::getController() const
{
	return pController;
}

uint32_t Scene::getBufferCount() const
{
	uint32_t bufferCount = 0;

	for (Model *pModel : models)
	{
		bufferCount += pModel->getBufferCount();
	}

	return 1 + bufferCount;
}

uint32_t Scene::getTextureCount() const
{
	uint32_t textureCount = 0;

	for (Model *pModel : models)
	{
		textureCount += pModel->getTextureCount();
	}

	return textureCount;
}

uint32_t Scene::getDecriptorSetCount() const
{
	uint32_t setCount = 1;

	for (Model *pModel : models)
	{
		setCount += 1 + pModel->getMeshCount();
	}

	return setCount;
}

void Scene::initDescriptorSets(DescriptorPool *pDescriptorPool)
{
	lightingDescriptorSet = pDescriptorPool->getDescriptorSet({ pLightingBuffer }, { }, true, lightingDSLayout);

	for (Model *pModel : models)
	{
		pModel->initDescriptorSets(pDescriptorPool);
	}
}

void Scene::createPipelines(RenderPass * pRenderPass)
{
	AssimpModel::createPipeline(pDevice, { lightingDSLayout }, pRenderPass);
	SkyboxModel::createPipeline(pDevice, { lightingDSLayout }, pRenderPass);
}

void Scene::updateScene()
{
	double deltaSec = frameTimer.getDeltaSec();

	pController->controlCamera(deltaSec);

	pSkybox->setModelMatrix(glm::translate(glm::mat4(1.0f), pCamera->getPos()));

	for (Model *pModel : models)
	{
		pModel->setViewMatrix(pCamera->getViewMatrix());
	}
}

void Scene::draw(VkCommandBuffer commandBuffer)
{
	for (Model *pModel : models)
	{
		pModel->draw(commandBuffer, { lightingDescriptorSet });
	}
}

void Scene::resizeExtent(RenderPass * pRenderPass)
{
	pCamera->setCameraExtent(pRenderPass->attachmentsExtent);

	for (Model *pModel : models)
	{
		pModel->setProjectionMatrix(pCamera->getProjectionMatrix());
	}

	AssimpModel::recreatePipeline(pRenderPass);
	SkyboxModel::recreatePipeline(pRenderPass);
}

// private:

void Scene::initCamera(VkExtent2D cameraExtent)
{
	glm::vec3 pos{ 0.0f, 0.0f, -30.0f };
	glm::vec3 forward{ 0.0f, 0.0f, 1.0f };
	glm::vec3 up{ 0.0f, -1.0f, 0.0f };

	pCamera = new Camera(pos, forward, up, cameraExtent);
}

void Scene::initLighting()
{
	lighting = Lighting{
		glm::vec3(1.0f, 1.0f, 1.0f),		// color
		0.6f,								// ambientStrength
		glm::vec3(-0.89f, 0.4f, -0.21f),	// direction
		0.6f,								// diffuseStrength
		pCamera->getPos(),					// cameraPos
		2.0f								// specularPower
	};

	pLightingBuffer = new Buffer(pDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(Lighting));
	pLightingBuffer->updateData(&lighting, sizeof(Lighting), 0);
}

void Scene::initModels()
{
	pMustang = new AssimpModel(pDevice, MUSTANG_FILE);
	glm::mat4 modelMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	modelMatrix = glm::scale(modelMatrix, glm::vec3(0.04f, 0.04f, 0.04f));
	pMustang->setModelMatrix(modelMatrix);

	pSkybox = new SkyboxModel(pDevice, SKYBOX_DIR, ".jpg");

	//models.push_back(pMustang);
	models.push_back(pSkybox);
	models.push_back(pMustang);

	for (Model *pModel : models)
	{
		pModel->setMvpMatrices({ pModel->getModelMatrix(), pCamera->getViewMatrix(), pCamera->getProjectionMatrix() });
	}
}
