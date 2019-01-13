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
	sceneDao.open(sceneFile);

	pCamera = new Camera(pDevice, cameraExtent, sceneDao.getCameraAttributes());
	pLighting = new Lighting(pDevice, sceneDao.getLightingAttributes(), shadowsDistance);
	skybox = new SkyboxModel(pDevice, sceneDao.getSkyboxInfo());
	terrain = new TerrainModel(pDevice, { 1.0f, 1.0f }, { 1000, 1000 }, sceneDao.getTerrainInfo());

	models = sceneDao.getModels(pDevice);
}

Scene::~Scene()
{
	for (const auto& pipeline : pipelines)
	{
		delete pipeline;
	}

	delete skybox;
	delete terrain;

	for (const auto& [key, model] : models)
	{
		delete model;
	}

    for (const auto& [key, descriptorStruct] : descriptors)
    {
		vkDestroyDescriptorSetLayout(pDevice->getVk(), descriptorStruct.layout, nullptr);
    }

	delete pLighting;
	delete pCamera;
	delete pSsaoKernel;
}

uint32_t Scene::getBufferCount() const
{
	uint32_t bufferCount = 9;

	bufferCount += skybox->getBufferCount();
	bufferCount += terrain->getBufferCount();

	for (const auto&[key, model] : models)
	{
		bufferCount += model->getBufferCount();
	}

	return bufferCount;
}

uint32_t Scene::getTextureCount() const
{
	uint32_t textureCount = 10;

	textureCount += skybox->getTextureCount();
	textureCount += terrain->getTextureCount();

	for (const auto&[key, model] : models)
	{
		textureCount += model->getTextureCount();
	}
	
	return textureCount;
}

uint32_t Scene::getDescriptorSetCount() const
{
	uint32_t setCount = uint32_t(FINAL) + 1;

	setCount += skybox->getDescriptorSetCount();
	setCount += terrain->getDescriptorSetCount();

	for (const auto&[key, model] : models)
	{
		setCount += model->getDescriptorSetCount();
	}

	return setCount;
}

Camera* Scene::getCamera() const
{
	return pCamera;
}

void Scene::prepareSceneRendering(DescriptorPool *pDescriptorPool, const RenderPassesMap &renderPasses)
{
	initDescriptorSets(pDescriptorPool, renderPasses);
	initPipelines(renderPasses);
	initStaticPipelines(renderPasses);
}

void Scene::updateScene()
{
	const double deltaSec = frameTimer.getDeltaSec();

	pCamera->move(deltaSec);
	pCamera->updateSpace();
	pLighting->update(pCamera->getPos());
	skybox->setTransformation(translate(glm::mat4(1.0f), pCamera->getPos()), 0);
}

void Scene::render(VkCommandBuffer commandBuffer, RenderPassType type)
{
    switch (type)
    {
    case DEPTH:
		for (const auto&[key, model] : models)
		{
			model->renderDepth(commandBuffer, { descriptors.at(DEPTH).set });
		}
        break;
    case GEOMETRY:
		for (const auto&[key, model] : models)
		{
			model->renderGeometry(commandBuffer, { descriptors.at(GEOMETRY).set });
		}
		terrain->renderGeometry(commandBuffer, { descriptors.at(GEOMETRY).set });
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
		skybox->renderFinal(commandBuffer, { descriptors.at(FINAL).set });
		for (const auto&[key, model] : models)
		{
			model->renderFinal(commandBuffer, { descriptors.at(FINAL).set });
		}
		terrain->renderFinal(commandBuffer, { descriptors.at(FINAL).set });
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

void Scene::initDescriptorSets(DescriptorPool *pDescriptorPool, RenderPassesMap renderPasses)
{
	DescriptorStruct descriptorStruct{};

    // Depth:

	descriptorStruct.layout = pDescriptorPool->createDescriptorSetLayout({ VK_SHADER_STAGE_VERTEX_BIT }, {});
	descriptorStruct.set = pDescriptorPool->getDescriptorSet(descriptorStruct.layout);
	pDescriptorPool->updateDescriptorSet(descriptorStruct.set, { pLighting->getSpaceBuffer() }, {});
	descriptors.insert({ DEPTH, descriptorStruct });

    // Geometry:

	descriptorStruct.layout = pDescriptorPool->createDescriptorSetLayout({ VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT }, {});
	descriptorStruct.set = pDescriptorPool->getDescriptorSet(descriptorStruct.layout);
	pDescriptorPool->updateDescriptorSet(descriptorStruct.set, { pCamera->getSpaceBuffer(), pLighting->getAttributesBuffer() }, {});
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

	skybox->initDescriptorSets(pDescriptorPool);
	terrain->initDescriptorSets(pDescriptorPool);
	for (const auto& [key, model] : models)
	{
		model->initDescriptorSets(pDescriptorPool);
	}
}

void Scene::initPipelines(RenderPassesMap renderPasses)
{
	const std::string SKYBOX_SHADERS_DIR = "Shaders/Skybox/";

    std::unordered_map<RenderPassType, std::string> shadersDirectories{
		{ DEPTH, "Shaders/Depth" },
		{ GEOMETRY, "Shaders/Geometry" },
		{ FINAL, "Shaders/Final" }
	};

	pipelines.push_back(skybox->createPipeline(
		{ descriptors.at(FINAL).layout },
		FINAL,
		renderPasses.at(FINAL),
		{
			std::make_shared<ShaderModule>(pDevice, File::getPath(SKYBOX_SHADERS_DIR, "Vert.spv"), VK_SHADER_STAGE_VERTEX_BIT),
			std::make_shared<ShaderModule>(pDevice, File::getPath(SKYBOX_SHADERS_DIR, "Frag.spv"), VK_SHADER_STAGE_FRAGMENT_BIT)
		}
	));

    for (const auto& shadersDir : shadersDirectories)
    {
		RenderPassType type = shadersDir.first;
		std::string directory = shadersDir.second;

		pipelines.push_back(terrain->createPipeline(
			{ descriptors.at(type).layout },
            type,
			renderPasses.at(type),
			{
				std::make_shared<ShaderModule>(pDevice, File::getPath(directory, "Vert.spv"), VK_SHADER_STAGE_VERTEX_BIT),
				std::make_shared<ShaderModule>(pDevice, File::getPath(directory, "Frag.spv"), VK_SHADER_STAGE_FRAGMENT_BIT)
			}
		));
        for (const auto& [key, model] : models)
        {
        	model->setPipeline(type, terrain->getPipeline(type));
        }
    }
}

void Scene::initStaticPipelines(RenderPassesMap renderPasses)
{
    const std::string FULLSCREEN_SHADERS_DIR = "Shaders/Fullscreen";
	const std::string SSAO_SHADERS_DIR = "Shaders/Ssao";
	const std::string SSAO_BLUR_SHADERS_DIR = "Shaders/SsaoBlur";
	const std::string LIGHTING_SHADERS_DIR = "Shaders/Lighting";

    const auto fullscreenVertexShader = std::make_shared<ShaderModule>(
        pDevice,
        File::getPath(FULLSCREEN_SHADERS_DIR, "Vert.spv"),
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
		pDevice,
		File::getPath(SSAO_SHADERS_DIR, "Frag.spv"),
		VK_SHADER_STAGE_FRAGMENT_BIT,
		ssaoConstantEntries,
		std::vector<const void*>{ &sampleCount, &pSsaoKernel->SIZE, &pSsaoKernel->RADIUS, &pSsaoKernel->POWER }
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
        pDevice,
        File::getPath(SSAO_BLUR_SHADERS_DIR, "Frag.spv"),
        VK_SHADER_STAGE_FRAGMENT_BIT,
        std::vector<VkSpecializationMapEntry>{ ssaoBlurConstantEntry },
        std::vector<const void*>{ &pSsaoKernel->BLUR_RADIUS }
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
        pDevice,
		File::getPath(LIGHTING_SHADERS_DIR, "Frag.spv"),
        VK_SHADER_STAGE_FRAGMENT_BIT, 
        std::vector<VkSpecializationMapEntry>{ lightingConstantEntry }, 
        std::vector<const void*>{ &sampleCount }
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
