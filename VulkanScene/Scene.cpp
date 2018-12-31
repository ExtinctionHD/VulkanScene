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

Scene::Scene(Device *pDevice, VkExtent2D cameraExtent, const std::string &sceneFile, float shadowsDistance)
    : pDevice(pDevice), pSsaoKernel(new SsaoKernel(pDevice))
{
    std::cout << "Creating scene..." << std::endl;

	sceneDao.open(sceneFile);

	pCamera = new Camera(pDevice, cameraExtent, sceneDao.getCameraAttributes());
	pLighting = new Lighting(pDevice, sceneDao.getLightingAttributes(), shadowsDistance);
	pSkybox = new SkyboxModel(pDevice, sceneDao.getSkyboxInfo());
	pTerrain = new TerrainModel(pDevice, { 1.0f, 1.0f }, { 1000, 1000 }, sceneDao.getTerrainInfo());
	pController = new Controller(pCamera);

	initModels();
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
	uint32_t bufferCount = 9;

	for (Model *pModel : models)
	{
		bufferCount += pModel->getBufferCount();
	}

	return bufferCount;
}

uint32_t Scene::getTextureCount() const
{
	uint32_t textureCount = 10;

	for (Model *pModel : models)
	{
		textureCount += pModel->getTextureCount();
	}
	
	return textureCount;
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
	const double deltaSec = frameTimer.getDeltaSec();

	pController->controlCamera(deltaSec);
	pLighting->update(pCamera->getPos());

	pSkybox->setTransformation(translate(glm::mat4(1.0f), pCamera->getPos()), 0);
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

void Scene::updateDescriptorSets(DescriptorPool *pDescriptorPool, RenderPassesMap renderPasses)
{
	// Ssao:

	GeometryRenderPass *pGeometryRenderPass = dynamic_cast<GeometryRenderPass*>(renderPasses.at(GEOMETRY));
	std::vector<TextureImage*> textures{
		pGeometryRenderPass->getPosMap(),
		pGeometryRenderPass->getNormalMap(),
		pSsaoKernel->getNoiseTexture()
	};
	pDescriptorPool->updateDescriptorSet(
		descriptors.at(SSAO).set,
		{ pSsaoKernel->getKernelBuffer(), pCamera->getSpaceBuffer() },
		textures
	);

	// Ssao blur:

	pDescriptorPool->updateDescriptorSet(
		descriptors.at(SSAO_BLUR).set,
		{},
		{ dynamic_cast<SsaoRenderPass*>(renderPasses.at(SSAO))->getSsaoMap() }
	);

	// Lighting:

	TextureImage *pShadowsMap = dynamic_cast<DepthRenderPass*>(renderPasses.at(DEPTH))->getDepthMap();
	textures = std::vector<TextureImage*>{
		pGeometryRenderPass->getPosMap(),
		pGeometryRenderPass->getNormalMap(),
		pGeometryRenderPass->getAlbedoMap(),
		dynamic_cast<SsaoRenderPass*>(renderPasses.at(SSAO_BLUR))->getSsaoMap(),
		pShadowsMap
	};
	pDescriptorPool->updateDescriptorSet(
		descriptors.at(LIGHTING).set,
		{ pLighting->getAttributesBuffer(), pLighting->getSpaceBuffer() },
		textures
	);
}

// private:

void Scene::initModels()
{
    const std::string REGERA_FILE = "models/Koenigsegg Regera/regera.fbx";
	const std::string HOUSE_FILE = "models/House/house.obj";

    pRegera = new AssimpModel(pDevice, REGERA_FILE, 4);

	pRegera->move({ -6.8f, 0.0f, 5.0 }, 1);
	pRegera->move({ -4.2f, 0.0f, 6.0 }, 2);
	pRegera->move({ 10.0f, 0.0f, 1.0 }, 3);

    pRegera->scale(glm::vec3(2.050f / pRegera->getBaseSize().x), 0);
	pRegera->scale(glm::vec3(2.050f / pRegera->getBaseSize().x), 1);
	pRegera->scale(glm::vec3(2.050f / pRegera->getBaseSize().x), 2);
	pRegera->scale(glm::vec3(2.050f / pRegera->getBaseSize().x), 3);

	pRegera->rotateAxisY(-20.0f, 1);
	pRegera->rotateAxisY(-5.0f, 2);
	pRegera->rotateAxisY(40.0f, 3);

    pRegera->rotateAxisX(90.0f, 0);
	pRegera->rotateAxisX(90.0f, 1);
	pRegera->rotateAxisX(90.0f, 2);
	pRegera->rotateAxisX(90.0f, 3);

    pRegera->optimizeMemory();

    pHouse = new AssimpModel(pDevice, HOUSE_FILE, 1);
    pHouse->move({ -2.1f, 0.14f, 3.0f }, 0);
    pHouse->rotateAxisX(180.0f, 0);
    pHouse->optimizeMemory();

	models.push_back(pSkybox);
	models.push_back(pRegera);
    models.push_back(pHouse);
	models.push_back(pTerrain);
}

void Scene::initDescriptorSets(DescriptorPool *pDescriptorPool, RenderPassesMap renderPasses)
{
	DescriptorStruct descriptorStruct{};

    // Depth:

	descriptorStruct.layout = pDescriptorPool->createDescriptorSetLayout({ VK_SHADER_STAGE_VERTEX_BIT }, {});
	descriptorStruct.set = pDescriptorPool->getDescriptorSet(descriptorStruct.layout);
	pDescriptorPool->updateDescriptorSet(descriptorStruct.set, { pLighting->getSpaceBuffer() }, {});
	descriptors.insert({ DEPTH, descriptorStruct });

    // Geometry:

	descriptorStruct.layout = pDescriptorPool->createDescriptorSetLayout({ VK_SHADER_STAGE_VERTEX_BIT }, {});
	descriptorStruct.set = pDescriptorPool->getDescriptorSet(descriptorStruct.layout);
	pDescriptorPool->updateDescriptorSet(descriptorStruct.set, { pCamera->getSpaceBuffer() }, {});
	descriptors.insert({ GEOMETRY, descriptorStruct });

    // Ssao:

    GeometryRenderPass *pGeometryRenderPass = dynamic_cast<GeometryRenderPass*>(renderPasses.at(GEOMETRY));
	std::vector<TextureImage*> textures{
		pGeometryRenderPass->getPosMap(),
		pGeometryRenderPass->getNormalMap(),
		pSsaoKernel->getNoiseTexture()
	};

	std::vector<VkShaderStageFlags> texturesShaderStages(textures.size(), VK_SHADER_STAGE_FRAGMENT_BIT);

	descriptorStruct.layout = pDescriptorPool->createDescriptorSetLayout(
	    {VK_SHADER_STAGE_FRAGMENT_BIT, VK_SHADER_STAGE_FRAGMENT_BIT},
	    texturesShaderStages
	);
	descriptorStruct.set = pDescriptorPool->getDescriptorSet(
	    descriptorStruct.layout
	);
	pDescriptorPool->updateDescriptorSet(
		descriptorStruct.set,
		{ pSsaoKernel->getKernelBuffer(), pCamera->getSpaceBuffer() },
		textures
	);
	descriptors.insert({ SSAO, descriptorStruct });

    // Ssao blur:

	descriptorStruct.layout = pDescriptorPool->createDescriptorSetLayout({}, { VK_SHADER_STAGE_FRAGMENT_BIT });
	descriptorStruct.set = pDescriptorPool->getDescriptorSet(
		descriptorStruct.layout
	);
	pDescriptorPool->updateDescriptorSet(
		descriptorStruct.set,
		{},
		{ dynamic_cast<SsaoRenderPass*>(renderPasses.at(SSAO))->getSsaoMap() }
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
	texturesShaderStages = std::vector<VkShaderStageFlags>(textures.size(), VK_SHADER_STAGE_FRAGMENT_BIT);

	descriptorStruct.layout = pDescriptorPool->createDescriptorSetLayout(
		{ VK_SHADER_STAGE_FRAGMENT_BIT, VK_SHADER_STAGE_FRAGMENT_BIT },
		texturesShaderStages
	);
	descriptorStruct.set = pDescriptorPool->getDescriptorSet(
		descriptorStruct.layout
	);
	pDescriptorPool->updateDescriptorSet(
		descriptorStruct.set,
		{ pLighting->getAttributesBuffer(), pLighting->getSpaceBuffer() },
		textures
	);
	descriptors.insert({ LIGHTING, descriptorStruct });

    // Final:

	descriptorStruct.layout = pDescriptorPool->createDescriptorSetLayout(
		{ VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT },
		{ VK_SHADER_STAGE_FRAGMENT_BIT }
	);
	descriptorStruct.set = pDescriptorPool->getDescriptorSet(
		descriptorStruct.layout
	);
	pDescriptorPool->updateDescriptorSet(
		descriptorStruct.set,
		{ pCamera->getSpaceBuffer(), pLighting->getSpaceBuffer(), pLighting->getAttributesBuffer() },
		{ pShadowsMap }
	);
	descriptors.insert({ FINAL, descriptorStruct });

	for (Model *pModel : models)
	{
		pModel->initDescriptorSets(pDescriptorPool);
	}
}

void Scene::initPipelines(RenderPassesMap renderPasses)
{
	const std::string SKYBOX_SHADERS_DIR = "shaders/skybox/";

    std::unordered_map<RenderPassType, std::string> shadersDirectories{
		{ DEPTH, "shaders/depth" },
		{ GEOMETRY, "shaders/geometry" },
		{ FINAL, "shaders/final" }
	};

	pipelines.push_back(pSkybox->createPipeline(
		{ descriptors.at(FINAL).layout },
		FINAL,
		renderPasses.at(FINAL),
		{
			std::make_shared<ShaderModule>(pDevice->device, File::getPath(SKYBOX_SHADERS_DIR, "vert.spv"), VK_SHADER_STAGE_VERTEX_BIT),
			std::make_shared<ShaderModule>(pDevice->device, File::getPath(SKYBOX_SHADERS_DIR, "frag.spv"), VK_SHADER_STAGE_FRAGMENT_BIT)
		}
	));

    for (const auto& shadersDir : shadersDirectories)
    {
		RenderPassType type = shadersDir.first;
		std::string directory = shadersDir.second;

		pipelines.push_back(pTerrain->createPipeline(
			{ descriptors.at(type).layout },
            type,
			renderPasses.at(type),
			{
				std::make_shared<ShaderModule>(pDevice->device, File::getPath(directory, "vert.spv"), VK_SHADER_STAGE_VERTEX_BIT),
				std::make_shared<ShaderModule>(pDevice->device, File::getPath(directory, "frag.spv"), VK_SHADER_STAGE_FRAGMENT_BIT)
			}
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
    const std::string FULLSCREEN_SHADERS_DIR = "shaders/fullscreen";
	const std::string SSAO_SHADERS_DIR = "shaders/ssao";
	const std::string SSAO_BLUR_SHADERS_DIR = "shaders/ssaoBlur";
	const std::string LIGHTING_SHADERS_DIR = "shaders/lighting";

    const auto fullscreenVertexShader = std::make_shared<ShaderModule>(
        pDevice->device,
        File::getPath(FULLSCREEN_SHADERS_DIR, "vert.spv"),
        VK_SHADER_STAGE_VERTEX_BIT
    );

    #pragma region Ssao

    const uint32_t SSAO_CONSTANT_COUNT = 4;
	std::vector<VkSpecializationMapEntry> ssaoConstantEntries;
	for (uint32_t i = 0; i < SSAO_CONSTANT_COUNT; i++)
	{
		ssaoConstantEntries.push_back({ i, sizeof(uint32_t) * i, sizeof(uint32_t) });
	}

	uint32_t sampleCount = renderPasses.at(GEOMETRY)->getSampleCount();
    const auto ssaoFragmentShader = std::make_shared<ShaderModule>(
		pDevice->device,
		File::getPath(SSAO_SHADERS_DIR, "frag.spv"),
		VK_SHADER_STAGE_FRAGMENT_BIT,
		ssaoConstantEntries,
		std::vector<const void *>{ &sampleCount, &pSsaoKernel->SIZE, &pSsaoKernel->RADIUS, &pSsaoKernel->POWER }
	);

	GraphicsPipeline *pSsaoPipeline = new GraphicsPipeline(
		pDevice,
		{ descriptors.at(SSAO).layout },
		renderPasses.at(SSAO),
		{ fullscreenVertexShader, ssaoFragmentShader },
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

    const VkSpecializationMapEntry ssaoBlurConstantEntry{
		0,                  // constantID
		0,                  // offset
		sizeof(uint32_t)    // size
	};

    const auto ssaoBlurFragmentShader = std::make_shared<ShaderModule>(
        pDevice->device,
        File::getPath(SSAO_BLUR_SHADERS_DIR, "frag.spv"),
        VK_SHADER_STAGE_FRAGMENT_BIT,
        std::vector<VkSpecializationMapEntry>{ ssaoBlurConstantEntry },
        std::vector<const void *>{ &pSsaoKernel->BLUR_RADIUS }
    );

	GraphicsPipeline *pSsaoBlurPipeline = new GraphicsPipeline(
		pDevice,
		{ descriptors.at(SSAO_BLUR).layout },
		renderPasses.at(SSAO_BLUR),
		{ fullscreenVertexShader, ssaoBlurFragmentShader },
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
		0,
		0,
		sizeof(uint32_t)
	};

    auto lightingFragmentShader = std::make_shared<ShaderModule>(
        pDevice->device,
		File::getPath(LIGHTING_SHADERS_DIR, "frag.spv"),
        VK_SHADER_STAGE_FRAGMENT_BIT, 
        std::vector<VkSpecializationMapEntry>{ lightingConstantEntry }, 
        std::vector<const void *>{ &sampleCount }
    );

	GraphicsPipeline *pLightingPipeline = new GraphicsPipeline(
		pDevice,
		{ descriptors.at(LIGHTING).layout },
		renderPasses.at(LIGHTING),
		{ fullscreenVertexShader, lightingFragmentShader },
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
