#include "AssimpModel.h"
#include "SkyboxModel.h"

#include "DepthRenderPass.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>
#include "GeometryRenderPass.h"

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
	for (GraphicsPipeline* pPipeline : pipelines)
	{
		delete pPipeline;
	}

	for (Model *pModel : models)
	{
		delete pModel;
	}

    for (auto descriptorStruct : descriptors)
    {
		vkDestroyDescriptorSetLayout(pDevice->device, descriptorStruct.second.layout, nullptr);
    }

	delete pLighting;
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
	uint32_t textureCount = 5;

	for (Model *pModel : models)
	{
		textureCount += pModel->getTextureCount();
	}

	return textureCount;
}

uint32_t Scene::getDescriptorSetCount() const
{
	uint32_t setCount = 4;

	for (Model *pModel : models)
	{
		setCount += pModel->getDescriptorSetCount();
	}

	return setCount;
}

void Scene::prepareSceneRendering(DescriptorPool *pDescriptorPool, const RenderPassesMap &renderPasses)
{
	initDescriptorSets(pDescriptorPool, renderPasses);
	initPipelines(renderPasses);
}

void Scene::updateScene()
{
	double deltaSec = frameTimer.getDeltaSec();

	pController->controlCamera(deltaSec);
	pLighting->update(pCamera->getPos());

	// pCar->setTransform(rotate(pCar->getTransform(), glm::radians(30.0f) * float(deltaSec), glm::vec3(0.0f, -1.0f, 0.0f)));
	pSkybox->setTransform(translate(glm::mat4(1.0f), pCamera->getPos()));
}

void Scene::renderDepth(VkCommandBuffer commandBuffer)
{
	for (Model *pModel : models)
	{
        if (pModel != pSkybox && pModel != pTerrain)
        {
			pModel->renderDepth(commandBuffer, { descriptors.at(DEPTH).set });
        }
	}
}

void Scene::renderGeometry(VkCommandBuffer commandBuffer)
{
	for (Model *pModel : models)
	{
		if (pModel != pSkybox)
		{
			pModel->renderGeometry(commandBuffer, { descriptors.at(GEOMETRY).set });
		}
	}
}

void Scene::renderLighting(VkCommandBuffer commandBuffer)
{
	for (Model *pModel : models)
	{
		if (pModel != pSkybox)
		{
			pModel->renderLighting(commandBuffer, { descriptors.at(LIGHTING).set });
		}
	}
}

void Scene::renderFinal(VkCommandBuffer commandBuffer)
{
	for (Model *pModel : models)
    {
		pModel->renderFinal(commandBuffer, { descriptors.at(FINAL).set });
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
	glm::vec3 pos{ 0.0f, -3.0f, -6.0f };
	glm::vec3 forward{ 0.0f, -0.8f, 1.0f };
	glm::vec3 up{ 0.0f, -1.0f, 0.0f };
	const float fov = 45.0f;

	pCamera = new Camera(pDevice, pos, forward, up, cameraExtent, fov);
}

void Scene::initLighting()
{
    // init lighting attributes

    // noon lighting attributes
	//Lighting::Attributes attributes{
	//	glm::vec3(1.0f, 1.0f, 1.0f),    // color
	//	0.8f,                           // ambientStrength
	//	glm::vec3(0.0f, 1.0f, -0.001f), // direction
	//	0.8f,                           // directedStrength
	//	pCamera->getPos(),              // cameraPos
	//	16.0f                           // specularPower
	//};

    // clouds lighting attributes
	Lighting::Attributes attributes{
	    glm::vec3(1.0f, 1.0f, 1.0f),        // color
	    0.7f,								// ambientStrength
	    glm::vec3(-0.89f, 0.4f, -0.21f),    // direction
	    0.7f,								// directedStrength
	    pCamera->getPos(),                  // cameraPos
	    8.0f                                // specularPower
	};

    // sunset lighting attributes
	//Lighting::Attributes attributes{
	//	glm::vec3(0.98f, 0.65f, 0.45f),     // color
	//	0.2f,								// ambientStrength
	//	glm::vec3(-0.7f, 0.09f, 0.7f),	    // direction (sunset skybox)
	//	0.6f,								// directedStrength
	//	pCamera->getPos(),					// cameraPos
	//	16.0f								// specularPower
	//};

	const float spaceRadius = 10.0f;

	pLighting = new Lighting(pDevice, attributes, spaceRadius);
}

void Scene::initModels()
{
	const std::string REGERA_FILE = File::getExeDir() + "models/regera/regera.obj";
	const std::string MERCEDES_FILE = File::getExeDir() + "models/amgGT/amgGT.obj";
	const std::string AUDI_FILE = File::getExeDir() + "models/audiA7/audiA7.obj";
	const std::string VULCAN_FILE = File::getExeDir() + "models/vulcan/vulcan.obj";

	const std::string SUNSET_SKYBOX_DIR = File::getExeDir() + "textures/skyboxSunset/";
	const std::string CLOUDS_SKYBOX_DIR = File::getExeDir() + "textures/skyboxClouds/";
	const std::string NOON_SKYBOX_DIR = File::getExeDir() + "textures/skyboxNoon/";

	const std::string ROCKY_TERRAIN_DIR = File::getExeDir() + "textures/rockyTerrain/";
    const std::string BRICKS_TERRAIN_DIR = File::getExeDir() + "textures/asphaltBricks/";

	// initialize car model
	pCar = new AssimpModel(pDevice, REGERA_FILE);

    // regera transformations
	pCar->scale(glm::vec3(2.050f / pCar->getBaseSize().x));
	pCar->rotateAxisX(90.0f);

	// mercedes transformations
	/*pCar->scale(glm::vec3(1.953f / pCar->getBaseSize().x));
	pCar->rotateAxisX(180.0f);*/

    // audi transformations
	/*pCar->move({ 0.0f, -0.690, 0.0f });
	pCar->scale(glm::vec3(2.118f / pCar->getBaseSize().x));
	pCar->rotateAxisX(90.0f);*/

    // aston martin transformations
	/*pCar->scale(glm::vec3(2.063f / pCar->getBaseSize().x));
	pCar->rotateAxisX(90.0f);*/

	// initialize skybox model
	pSkybox = new SkyboxModel(pDevice, CLOUDS_SKYBOX_DIR, ".jpg");

	// initialize terrain model
	pTerrain = new TerrainModel(pDevice, { 2000, 2000 }, { 2000, 2000 }, ROCKY_TERRAIN_DIR, ".jpg");

	models.push_back(pSkybox);
	models.push_back(pTerrain);
	models.push_back(pCar);
}

void Scene::initDescriptorSets(DescriptorPool *pDescriptorPool, RenderPassesMap renderPasses)
{
	descriptors.insert({ DEPTH, {} });
	descriptors.at(DEPTH).set = pDescriptorPool->getDescriptorSet(
		{ pLighting->getSpaceBuffer() },
		{ },
		true,
		descriptors.at(DEPTH).layout
	);

	descriptors.insert({ GEOMETRY, {} });
	descriptors.at(GEOMETRY).set = pDescriptorPool->getDescriptorSet(
		{ pCamera->getSpaceBuffer(), pLighting->getSpaceBuffer() },
		{ },
		true,
		descriptors.at(GEOMETRY).layout
	);

    std::vector<TextureImage*> maps = dynamic_cast<GeometryRenderPass*>(renderPasses.at(GEOMETRY))->getMaps();
	maps.push_back(dynamic_cast<DepthRenderPass*>(renderPasses.at(DEPTH))->getDepthMap());

	descriptors.insert({ LIGHTING, {} });
	descriptors.at(LIGHTING).set = pDescriptorPool->getDescriptorSet(
		{ pLighting->getAttributesBuffer() },
		maps, 
		true, 
		descriptors.at(LIGHTING).layout
	);

	descriptors.insert({ FINAL, {} });
	descriptors.at(FINAL).set = pDescriptorPool->getDescriptorSet(
		{ pCamera->getSpaceBuffer(), pLighting->getSpaceBuffer() },
		{ },
		true,
		descriptors.at(FINAL).layout
	);

	for (Model *pModel : models)
	{
		pModel->initDescriptorSets(pDescriptorPool);
	}
}

void Scene::initPipelines(RenderPassesMap renderPasses)
{
	const std::string SKYBOX_SHADERS_DIR = File::getExeDir() + "shaders/skybox/";

    std::unordered_map<RenderPassType, std::string> shadersDirectories{
		{ DEPTH, File::getExeDir() + "shaders/depth/" },
		{ GEOMETRY, File::getExeDir() + "shaders/geometry/" },
		{ LIGHTING, File::getExeDir() + "shaders/lighting/" },
		{ FINAL, File::getExeDir() + "shaders/final/" }
	};

    for (auto shadersDir : shadersDirectories)
    {
		RenderPassType type = shadersDir.first;
		std::string directory = shadersDir.second;

		std::vector<ShaderModule*> shaders;
		shaders.push_back(new ShaderModule(pDevice->device, directory + "vert.spv", VK_SHADER_STAGE_VERTEX_BIT));
        if (type == LIGHTING)
        {
            VkSpecializationMapEntry entry{
			    0,                  // constantID
			    0,                  // offset
			    sizeof(uint32_t)    // size
			};
			uint32_t sampleCount = pDevice->getSampleCount();
			shaders.push_back(new ShaderModule(pDevice->device, directory + "frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT, { entry }, { &sampleCount }));
        } 
        else
		{
		    shaders.push_back(new ShaderModule(pDevice->device, directory + "frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT));
		}

		pipelines.push_back(pCar->createPipeline(
			{ descriptors.at(type).layout },
            type,
			renderPasses.at(type),
            shaders
		));
		pTerrain->setPipeline(type, pCar->getPipeline(type));
    }

	pipelines.push_back(pSkybox->createPipeline(
		{ descriptors.at(FINAL).layout },
        FINAL,
		renderPasses.at(FINAL),
		{
			new ShaderModule(pDevice->device, SKYBOX_SHADERS_DIR + "vert.spv", VK_SHADER_STAGE_VERTEX_BIT),
			new ShaderModule(pDevice->device, SKYBOX_SHADERS_DIR + "frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT)
		}
	));

}
