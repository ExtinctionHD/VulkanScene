#include "AssimpModel.h"
#include "SkyboxModel.h"
#include <glm/gtc/matrix_transform.hpp>

#include "Scene.h"
#include "ShadowsRenderPass.h"

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
	for (GraphicsPipeline* pPipeline : pipelines)
	{
		delete pPipeline;
	}

	for (Model *pModel : models)
	{
		delete pModel;
	}

	vkDestroyDescriptorSetLayout(pDevice->device, shadowsDsLayout, nullptr);
	vkDestroyDescriptorSetLayout(pDevice->device, sceneDsLayout, nullptr);

	delete pLightingBuffer;
	delete pCamera;
	delete pController;
}

Controller* Scene::getController() const
{
	return pController;
}

uint32_t Scene::getBufferCount() const
{
	uint32_t bufferCount = 2;

	for (Model *pModel : models)
	{
		bufferCount += pModel->getBufferCount();
	}

	return bufferCount;
}

uint32_t Scene::getTextureCount() const
{
	uint32_t textureCount = 1;

	for (Model *pModel : models)
	{
		textureCount += pModel->getTextureCount();
	}

	return textureCount;
}

uint32_t Scene::getDescriptorSetCount() const
{
	uint32_t setCount = 2;

	for (Model *pModel : models)
	{
		setCount += pModel->getDescriptorSetCount();
	}

	return setCount;
}

void Scene::prepareSceneRendering(DescriptorPool *pDescriptorPool, RenderPassesMap renderPasses)
{
	shadowDescriptorSet = pDescriptorPool->getDescriptorSet({ pCamera->getViewProjBuffer() }, { }, true, shadowsDsLayout);

	TextureImage *pShadowsMap = dynamic_cast<ShadowsRenderPass*>(renderPasses.at(shadows))->getShadowsMap();
	sceneDescriptorSet = pDescriptorPool->getDescriptorSet({ pCamera->getViewProjBuffer(), pLightingBuffer, }, { pShadowsMap }, true, sceneDsLayout);

	for (Model *pModel : models)
	{
		pModel->initDescriptorSets(pDescriptorPool);
	}

	initPipelines(renderPasses);
}

void Scene::updateScene()
{
	double deltaSec = frameTimer.getDeltaSec();

	pController->controlCamera(deltaSec);

	pSkybox->setTransform(glm::translate(glm::mat4(1.0f), pCamera->getPos()));

	lighting.cameraPos = pCamera->getPos();
	pLightingBuffer->updateData(&lighting.cameraPos, sizeof(lighting.cameraPos), offsetof(Lighting, cameraPos));
}

void Scene::drawShadows(VkCommandBuffer commandBuffer)
{
	for (Model *pModel : models)
	{
		pModel->drawShadows(commandBuffer, { shadowDescriptorSet }, pShadowsPipeline);
	}
}

void Scene::draw(VkCommandBuffer commandBuffer)
{
	for (Model *pModel : models)
	{
		pModel->draw(commandBuffer, { sceneDescriptorSet });
	}
}

void Scene::resizeExtent(VkExtent2D newExtent)
{
	pCamera->setExtent(newExtent);

	for (GraphicsPipeline* pPipeline : pipelines)
	{
		pPipeline->recreate();
	}
}

// private:

void Scene::initCamera(VkExtent2D cameraExtent)
{
	glm::vec3 pos{ 0.0f, -40.0f, -80.0f };
	glm::vec3 forward{ 0.0f, -0.8f, 1.0f };
	glm::vec3 up{ 0.0f, -1.0f, 0.0f };

	pCamera = new Camera(pDevice, pos, forward, up, cameraExtent);
}

void Scene::initLighting()
{
	lighting = Lighting{
		glm::vec3(1.0f, 1.0f, 1.0f),		// color
		0.8f,								// ambientStrength
		glm::vec3(-0.89f, 0.4f, -0.21f),	// direction
		0.7f,								// diffuseStrength
		pCamera->getPos(),					// cameraPos
		2.0f								// specularPower
	};

	pLightingBuffer = new Buffer(pDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(Lighting));
	pLightingBuffer->updateData(&lighting, sizeof(Lighting), 0);
}

void Scene::initModels()
{
	const std::string AVENTADOR_FILE = File::getExeDir() + "models/aventador/lamborghini_aventador.fbx";
	const std::string MUSTANG_FILE = File::getExeDir() + "models/mustangGT/mustang_GT.obj";
	const std::string FORD_FILE = File::getExeDir() + "models/fordGT/Ford GT 2017.obj";
	const std::string SKYBOX_DIR = File::getExeDir() + "textures/skyboxClouds/";
	const std::string TERRAIN_DIR = File::getExeDir() + "textures/asphaltBricks/";

	// initialize car model
	pCar = new AssimpModel(pDevice, FORD_FILE);
	glm::mat4 transform = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	transform = glm::scale(transform, glm::vec3(0.15f, 0.15f, 0.15f));
	pCar->setTransform(transform);

	// initalize skybox model
	pSkybox = new SkyboxModel(pDevice, SKYBOX_DIR, ".jpg");

	// initialize terrain model
	pTerrain = new TerrainModel(pDevice, { 20000, 20000 }, { 1000, 1000 }, TERRAIN_DIR, ".jpg");

	models.push_back(pSkybox);
	models.push_back(pTerrain);
	models.push_back(pCar);
}

void Scene::initPipelines(RenderPassesMap renderPasses)
{
	const std::string SHADOWS_SHADERS_DIR = File::getExeDir() + "shaders/shadows/";
	const std::string CAR_SHADERS_DIR = File::getExeDir() + "shaders/car/";
	const std::string SKY_SHADERS_DIR = File::getExeDir() + "shaders/skybox/";

    // create pipeline for rendering shadows
	const uint32_t inputBinding = 0;
	pShadowsPipeline = new GraphicsPipeline(
		pDevice->device,
		{ shadowsDsLayout, Model::getTransformDsLayout() },
		renderPasses.at(shadows),
		{
			new ShaderModule(pDevice->device, SHADOWS_SHADERS_DIR + "vert.spv", VK_SHADER_STAGE_VERTEX_BIT),
			new ShaderModule(pDevice->device, SHADOWS_SHADERS_DIR + "frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT)
		},
		Vertex::getBindingDescription(inputBinding),
		Vertex::getAttributeDescriptions(inputBinding)
	);
	pipelines.push_back(pShadowsPipeline);

    // create main pipeline
	pipelines.push_back(pCar->createPipeline(
		{ sceneDsLayout },
		renderPasses.at(final),
		{
			new ShaderModule(pDevice->device, CAR_SHADERS_DIR + "vert.spv", VK_SHADER_STAGE_VERTEX_BIT),
			new ShaderModule(pDevice->device, CAR_SHADERS_DIR + "frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT)
		}
	));
	pTerrain->setPipeline(*--pipelines.end());

    // create pipeline for skybox rendering
	pipelines.push_back(pSkybox->createPipeline(
		{ sceneDsLayout },
		renderPasses.at(final),
		{
			new ShaderModule(pDevice->device, SKY_SHADERS_DIR + "vert.spv", VK_SHADER_STAGE_VERTEX_BIT),
			new ShaderModule(pDevice->device, SKY_SHADERS_DIR + "frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT)
		}
	));

}
