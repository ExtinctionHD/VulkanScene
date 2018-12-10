#include "AssimpModel.h"
#include "SkyboxModel.h"
#include "GeometryRenderPass.h"
#include "Scene.h"
#include <iostream>
#include "DepthRenderPass.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>

#include "SsaoRenderPass.h"

// public:

Scene::Scene(Device *pDevice, VkExtent2D cameraExtent, const std::string &lightingFile, float shadowsDistance, std::vector<bool> modelsExistence)
    : pDevice(pDevice), pSsaoKernel(new SsaoKernel(pDevice))
{
    std::cout << "Creating scene..." << std::endl;

	initCamera(cameraExtent);
	initLighting(lightingFile, shadowsDistance);
	initModels(modelsExistence);

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
	delete pSsaoKernel;
}

Controller* Scene::getController() const
{
	return pController;
}

uint32_t Scene::getBufferCount() const
{
	uint32_t bufferCount = 3;

	for (Model *pModel : models)
	{
		bufferCount += pModel->getBufferCount();
	}

    // TODO
	return 0;
}

uint32_t Scene::getTextureCount() const
{
	uint32_t textureCount = 6;

	for (Model *pModel : models)
	{
		textureCount += pModel->getTextureCount();
	}

	// TODO
	return 0;
}

uint32_t Scene::getDescriptorSetCount() const
{
	uint32_t setCount = uint32_t(FINAL) + 1;

	for (Model *pModel : models)
	{
		setCount += pModel->getDescriptorSetCount();
	}

	return setCount;
}

void Scene::prepareSceneRendering(DescriptorPool *pDescriptorPool, const RenderPassesMap &renderPasses)
{\
    std::cout << "Preparing scene rendering..." << std::endl;

	initDescriptorSets(pDescriptorPool, renderPasses);
	initPipelines(renderPasses);
	initStaticPipelines(renderPasses);
}

void Scene::updateScene()
{
	double deltaSec = frameTimer.getDeltaSec();

	pController->controlCamera(deltaSec);
	pLighting->update(pCamera->getPos());

	pSkybox->setTransform(translate(glm::mat4(1.0f), pCamera->getPos()));
}

void Scene::render(VkCommandBuffer commandBuffer, RenderPassType type)
{
    switch (type)
    {
    case DEPTH:
		for (Model *pModel : models)
		{
			if (pModel != pSkybox && pModel != pTerrain)
			{
				pModel->renderDepth(commandBuffer, { descriptors.at(DEPTH).set });
			}
		}
        break;
    case GEOMETRY:
		for (Model *pModel : models)
		{
			if (pModel != pSkybox)
			{
				pModel->renderGeometry(commandBuffer, { descriptors.at(GEOMETRY).set });
			}
		}
        break;
    case SSAO:
		Model::renderFullscreenQuad(commandBuffer, { descriptors.at(SSAO).set }, SSAO);
        break;
	case SSAO_BLUR:
		Model::renderFullscreenQuad(commandBuffer, { descriptors.at(SSAO_BLUR).set }, SSAO_BLUR);
		break;
    case LIGHTING:
		Model::renderFullscreenQuad(commandBuffer, { descriptors.at(LIGHTING).set }, LIGHTING);
        break;
    case FINAL:
		for (Model *pModel : models)
		{
			pModel->renderFinal(commandBuffer, { descriptors.at(FINAL).set });
		}
        break;
    default:
		throw std::invalid_argument("Can't render scene for this type");
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

void Scene::initLighting(const std::string &lightingFile, float shadowsDistance)
{
	std::string skyboxDir;
	std::string skyboxExtension;
	pLighting = new Lighting(pDevice, File::getLightingAttributes(lightingFile, skyboxDir, skyboxExtension), shadowsDistance);

	pSkybox = new SkyboxModel(pDevice, skyboxDir, skyboxExtension);
	models.push_back(pSkybox);
}

void Scene::initModels(std::vector<bool> modelsExistence)
{
	const std::string AMG_GT_FILE = File::getExeDir() + "models/Mercedes AMG GT/amgGt.fbx";
	const std::string S63_FILE = File::getExeDir() + "models/Mercedes S63/s63.fbx";;
    const std::string REGERA_FILE = File::getExeDir() + "models/Koenigsegg Regera/regera.fbx";
    const std::string VULCAN_FILE = File::getExeDir() + "models/Aston Martin Vulcan/vulcan.fbx";
	const std::string HOUSE_FILE = File::getExeDir() + "models/House/house.obj";
    const std::string SUGAR_MARPLE_FILE = File::getExeDir() + "models/Trees/sugarMarple.obj";
	const std::string NORWAY_MARPLE_FILE = File::getExeDir() + "models/Trees/norwayMarple.obj";

    const int MODELS_GROUP_COUNT = 4;
    assert(modelsExistence.size() >= MODELS_GROUP_COUNT);

    if (modelsExistence[0])
    {
        std::cout << "Loading Mercedes AMG GT model..." << std::endl;

        pAmgGt = new AssimpModel(pDevice, AMG_GT_FILE);
        pAmgGt->move({ -4.2f, 0.0f, 7.0 });
        pAmgGt->scale(glm::vec3(1.953f / pAmgGt->getBaseSize().x));
        pAmgGt->rotateAxisY(-5.0f);
        pAmgGt->rotateAxisX(180.0f);
        pAmgGt->optimizeMemory();
        models.push_back(pAmgGt);

        std::cout << "Loading Mercedes S63 AMG model..." << std::endl;

        pS63 = new AssimpModel(pDevice, S63_FILE);
        pS63->move({ -5.5f, 0.0f, 4.5 });
        pS63->scale(glm::vec3(1.913f / pS63->getBaseSize().x));
        pS63->rotateAxisY(-20.0f);
        pS63->rotateAxisX(180.0f);
        pS63->optimizeMemory();
        models.push_back(pS63);
    }

    if (modelsExistence[1])
    {
        std::cout << "Loading Koenigsegg Regera model..." << std::endl;

        pRegera = new AssimpModel(pDevice, REGERA_FILE);
        pRegera->scale(glm::vec3(2.050f / pRegera->getBaseSize().x));
        pRegera->rotateAxisX(90.0f);
        pRegera->optimizeMemory();
        models.push_back(pRegera);

        std::cout << "Loading Aston Martin Vulcan model..." << std::endl;

        pVulcan = new AssimpModel(pDevice, VULCAN_FILE);
        pVulcan->move({ 10.0f, 0.0f, 1.0 });
        pVulcan->scale(glm::vec3(2.063f / pVulcan->getBaseSize().x));
        pVulcan->rotateAxisY(40.0f);
        pVulcan->rotateAxisX(90.0f);
        pVulcan->optimizeMemory();
        models.push_back(pVulcan);
    }

    if (modelsExistence[2])
    {
        std::cout << "Loading house model..." << std::endl;

        pHouse = new AssimpModel(pDevice, HOUSE_FILE);
        pHouse->move({ -2.1f, 0.14f, 3.0f });
        pHouse->rotateAxisX(180.0f);
        pHouse->optimizeMemory();
        models.push_back(pHouse);
    }

    if (modelsExistence[3])
    {
        std::cout << "Loading Sugar marple model..." << std::endl;

        pSugarMarple = new AssimpModel(pDevice, SUGAR_MARPLE_FILE);
        pSugarMarple->move({ 11.0f, 0.0f, -1.0 });
        pSugarMarple->scale(glm::vec3(16.0f / pSugarMarple->getBaseSize().y));
        pSugarMarple->rotateAxisX(180.0f);
        pSugarMarple->optimizeMemory();
        models.push_back(pSugarMarple);

        std::cout << "Loading Norway marple model..." << std::endl;

        pNorwayMarple = new AssimpModel(pDevice, NORWAY_MARPLE_FILE);
        pNorwayMarple->move({ -8.0f, 0.0f, 16.0 });
        pNorwayMarple->scale(glm::vec3(20.0f / pNorwayMarple->getBaseSize().y));
        pNorwayMarple->rotateAxisX(180.0f);
        pNorwayMarple->optimizeMemory();
        models.push_back(pNorwayMarple);
    }

	const std::string GRASS_TERRAIN_DIR = File::getExeDir() + "textures/grass/";
	const std::string ROCKY_TERRAIN_DIR = File::getExeDir() + "textures/rockyTerrain/";
	const std::string BRICKS_TERRAIN_DIR = File::getExeDir() + "textures/asphaltBricks/";

    std::cout << "Generating terrain model..." << std::endl;

	pTerrain = new TerrainModel(pDevice, { 1000, 1000 }, { 1000, 1000 }, GRASS_TERRAIN_DIR, ".jpg");
	models.push_back(pTerrain);
}

void Scene::initDescriptorSets(DescriptorPool *pDescriptorPool, RenderPassesMap renderPasses)
{
	DescriptorStruct descriptorStruct{};

    // Depth:

	descriptorStruct.layout = pDescriptorPool->createDescriptorSetLayout({ VK_SHADER_STAGE_VERTEX_BIT }, {});
	descriptorStruct.set = pDescriptorPool->getDescriptorSet({ pLighting->getSpaceBuffer() }, {}, descriptorStruct.layout);
	descriptors.insert({ DEPTH, descriptorStruct });

    // Geometry:

	descriptorStruct.layout = pDescriptorPool->createDescriptorSetLayout({ VK_SHADER_STAGE_VERTEX_BIT }, {});
	descriptorStruct.set = pDescriptorPool->getDescriptorSet({ pCamera->getSpaceBuffer() }, {}, descriptorStruct.layout);
	descriptors.insert({ GEOMETRY, descriptorStruct });

    // Ssao:

    GeometryRenderPass *pGeometryRenderPass = dynamic_cast<GeometryRenderPass*>(renderPasses.at(GEOMETRY));
	std::vector<TextureImage*> textures{
		pGeometryRenderPass->getPosMap(),
		pGeometryRenderPass->getNormalMap(),
		pSsaoKernel->getNoiseTexture()
	};

	std::vector<VkShaderStageFlagBits> texturesShaderStages(textures.size(), VK_SHADER_STAGE_FRAGMENT_BIT);

	descriptorStruct.layout = pDescriptorPool->createDescriptorSetLayout(
	    {VK_SHADER_STAGE_FRAGMENT_BIT, VK_SHADER_STAGE_FRAGMENT_BIT},
	    texturesShaderStages
	);
	descriptorStruct.set = pDescriptorPool->getDescriptorSet(
	    {pSsaoKernel->getKernelBuffer(), pCamera->getSpaceBuffer()},
	    textures,
	    descriptorStruct.layout
	);
	descriptors.insert({ SSAO, descriptorStruct });

    // Ssao blur:

	descriptorStruct.layout = pDescriptorPool->createDescriptorSetLayout({}, { VK_SHADER_STAGE_FRAGMENT_BIT });
	descriptorStruct.set = pDescriptorPool->getDescriptorSet(
		{},
        { dynamic_cast<SsaoRenderPass*>(renderPasses.at(SSAO))->getSsaoMap() },
		descriptorStruct.layout
	);
	descriptors.insert({ SSAO_BLUR, descriptorStruct });

    // Lighting:

	TextureImage *pShadowsMap = dynamic_cast<DepthRenderPass*>(renderPasses.at(DEPTH))->getDepthMap();
	textures = std::vector<TextureImage*>{
		pGeometryRenderPass->getPosMap(),
		pGeometryRenderPass->getNormalMap(),
		pGeometryRenderPass->getAlbedoMap(),
		dynamic_cast<SsaoRenderPass*>(renderPasses.at(SSAO_BLUR))->getSsaoMap(),
		pShadowsMap
	};
	texturesShaderStages = std::vector<VkShaderStageFlagBits>(textures.size(), VK_SHADER_STAGE_FRAGMENT_BIT);

	descriptorStruct.layout = pDescriptorPool->createDescriptorSetLayout(
		{ VK_SHADER_STAGE_FRAGMENT_BIT, VK_SHADER_STAGE_FRAGMENT_BIT },
		texturesShaderStages
	);
	descriptorStruct.set = pDescriptorPool->getDescriptorSet(
		{ pLighting->getAttributesBuffer(), pLighting->getSpaceBuffer() },
		textures,
		descriptorStruct.layout
	);
	descriptors.insert({ LIGHTING, descriptorStruct });

    // Final:

	descriptorStruct.layout = pDescriptorPool->createDescriptorSetLayout(
		{ VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT },
		{ VK_SHADER_STAGE_FRAGMENT_BIT }
	);
	descriptorStruct.set = pDescriptorPool->getDescriptorSet(
		{ pCamera->getSpaceBuffer(), pLighting->getSpaceBuffer(), pLighting->getAttributesBuffer() },
		{ pShadowsMap },
		descriptorStruct.layout
	);
	descriptors.insert({ FINAL, descriptorStruct });

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
		{ FINAL, File::getExeDir() + "shaders/final/" }
	};

	pipelines.push_back(pSkybox->createPipeline(
		{ descriptors.at(FINAL).layout },
		FINAL,
		renderPasses.at(FINAL),
		{
			new ShaderModule(pDevice->device, SKYBOX_SHADERS_DIR + "vert.spv", VK_SHADER_STAGE_VERTEX_BIT),
			new ShaderModule(pDevice->device, SKYBOX_SHADERS_DIR + "frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT)
		}
	));

    for (auto shadersDir : shadersDirectories)
    {
		RenderPassType type = shadersDir.first;
		std::string directory = shadersDir.second;

		std::vector<ShaderModule*> shaders;
		shaders.push_back(new ShaderModule(pDevice->device, directory + "vert.spv", VK_SHADER_STAGE_VERTEX_BIT));
        shaders.push_back(new ShaderModule(pDevice->device, directory + "frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT));

		pipelines.push_back(pTerrain->createPipeline(
			{ descriptors.at(type).layout },
            type,
			renderPasses.at(type),
            shaders
		));
        for (auto model : models)
        {
            if (model != pSkybox)
            {
				model->setPipeline(type, pTerrain->getPipeline(type));
            }
        }
    }
}

void Scene::initStaticPipelines(RenderPassesMap renderPasses)
{
    const std::string FULLSCREEN_SHADERS_DIR = File::getExeDir() + "shaders/fullscreen/";
	const std::string SSAO_SHADERS_DIR = File::getExeDir() + "shaders/ssao/";
	const std::string SSAO_BLUR_SHADERS_DIR = File::getExeDir() + "shaders/ssaoBlur/";
	const std::string LIGHTING_SHADERS_DIR = File::getExeDir() + "shaders/lighting/";

    ShaderModule *pFullscreenVertexShader = new ShaderModule(
        pDevice->device,
        FULLSCREEN_SHADERS_DIR + "vert.spv",
        VK_SHADER_STAGE_VERTEX_BIT
    );

    #pragma region Ssao

	uint32_t SSAO_CONSTANT_COUNT = 4;
	std::vector<VkSpecializationMapEntry> ssaoConstantEntries;
	for (uint32_t i = 0; i < SSAO_CONSTANT_COUNT; i++)
	{
		ssaoConstantEntries.push_back({ i, sizeof(uint32_t) * i, sizeof(uint32_t) });
	}

	uint32_t sampleCount = renderPasses.at(GEOMETRY)->getSampleCount();
	ShaderModule *pSsaoFragmentShader = new ShaderModule(
		pDevice->device,
		SSAO_SHADERS_DIR + "frag.spv",
		VK_SHADER_STAGE_FRAGMENT_BIT,
		ssaoConstantEntries,
		{ &sampleCount, &pSsaoKernel->SIZE, &pSsaoKernel->RADIUS, &pSsaoKernel->POWER }
	);

	GraphicsPipeline *pSsaoPipeline = new GraphicsPipeline(
		pDevice,
		{ descriptors.at(SSAO).layout },
		renderPasses.at(SSAO),
		{ pFullscreenVertexShader, pSsaoFragmentShader },
		{},
		{},
		renderPasses.at(SSAO)->getSampleCount(),
		renderPasses.at(SSAO)->getColorAttachmentCount(),
		VK_FALSE
	);
	Model::setStaticPipeline(SSAO, pSsaoPipeline);
	pipelines.push_back(pSsaoPipeline);

    #pragma endregion 

    #pragma region SsaoBlur

	VkSpecializationMapEntry ssaoBlurConstantEntry{
		0,                  // constantID
		0,                  // offset
		sizeof(uint32_t)    // size
	};

    ShaderModule *pSsaoBlurFragmentShader = new ShaderModule(
        pDevice->device,
        SSAO_BLUR_SHADERS_DIR + "frag.spv",
        VK_SHADER_STAGE_FRAGMENT_BIT,
        { ssaoBlurConstantEntry },
        { &pSsaoKernel->BLUR_RADIUS }
    );

	GraphicsPipeline *pSsaoBlurPipeline = new GraphicsPipeline(
		pDevice,
		{ descriptors.at(SSAO_BLUR).layout },
		renderPasses.at(SSAO_BLUR),
		{ pFullscreenVertexShader, pSsaoBlurFragmentShader },
		{},
		{},
		renderPasses.at(SSAO_BLUR)->getSampleCount(),
		renderPasses.at(SSAO_BLUR)->getColorAttachmentCount(),
		VK_FALSE
	);
	Model::setStaticPipeline(SSAO_BLUR, pSsaoBlurPipeline);
	pipelines.push_back(pSsaoBlurPipeline);

    #pragma endregion 

    #pragma region Lighting

	VkSpecializationMapEntry lightingConstantEntry{
		0,                  // constantID
		0,                  // offset
		sizeof(uint32_t)    // size
	};

    ShaderModule *pLightingFragmentShader = new ShaderModule(
        pDevice->device,
        LIGHTING_SHADERS_DIR + "frag.spv", 
        VK_SHADER_STAGE_FRAGMENT_BIT, 
        { lightingConstantEntry }, 
        { &sampleCount }
    );

	GraphicsPipeline *pLightingPipeline = new GraphicsPipeline(
		pDevice,
		{ descriptors.at(LIGHTING).layout },
		renderPasses.at(LIGHTING),
		{ pFullscreenVertexShader, pLightingFragmentShader },
		{},
		{},
		renderPasses.at(LIGHTING)->getSampleCount(),
		renderPasses.at(LIGHTING)->getColorAttachmentCount(),
		VK_FALSE
	);
	Model::setStaticPipeline(LIGHTING, pLightingPipeline);
	pipelines.push_back(pLightingPipeline);

    #pragma endregion 
}
