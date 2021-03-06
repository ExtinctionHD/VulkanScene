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

Scene::Scene(Device *device, VkExtent2D cameraExtent, const std::string &path)  : device(device)
{
	sceneDao.open(path);

	camera = new Camera(device, cameraExtent, sceneDao.getCameraAttributes());
	lighting = new Lighting(device, sceneDao.getLightingAttributes());
	skybox = new SkyboxModel(device, sceneDao.getSkyboxInfo());
	terrain = new TerrainModel(device, { 1.0f, 1.0f }, { 1000, 1000 }, sceneDao.getTerrainInfo());

	ssaoKernel = new SsaoKernel(device);
	pssmKernel = new PssmKernel(device, camera, lighting->getDirection());

	models = sceneDao.getModels(device);
}

Scene::~Scene()
{
	for (auto pipeline : pipelines)
	{
		delete pipeline;
	}

	delete skybox;
	delete terrain;
	for (const auto &[key, model] : models)
	{
		delete model;
	}

    for (auto [key, descriptorStruct] : descriptors)
    {
		vkDestroyDescriptorSetLayout(device->get(), descriptorStruct.layout, nullptr);
    }

	delete lighting;
	delete camera;
	delete ssaoKernel;
	delete pssmKernel;
}

uint32_t Scene::getBufferCount() const
{
	uint32_t bufferCount = 10;

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
	return camera;
}

void Scene::prepareSceneRendering(DescriptorPool *descriptorPool, const RenderPassesMap &renderPasses)
{
	initDescriptorSets(descriptorPool, renderPasses);
	initPipelines(renderPasses);
	initStaticPipelines(renderPasses);
}

void Scene::updateScene()
{
	const float deltaSec = frameTimer.getDeltaSec();

	camera->move(deltaSec);
	camera->updateSpace();

	lighting->update(camera->getPos());

	pssmKernel->update();

	skybox->setTransformation(translate(glm::mat4(1.0f), camera->getPos()), 0);
}

void Scene::render(VkCommandBuffer commandBuffer, RenderPassType type, uint32_t renderIndex)
{
    switch (type)
    {
    case DEPTH:
		for (const auto&[key, model] : models)
		{
			model->renderDepth(commandBuffer, { descriptors.at(DEPTH).set }, renderIndex);
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
		Model::renderFullscreenQuad(commandBuffer, SSAO, { descriptors.at(SSAO).set });
        break;
	case SSAO_BLUR:
		Model::renderFullscreenQuad(commandBuffer, SSAO_BLUR, { descriptors.at(SSAO_BLUR).set });
		break;
    case LIGHTING:
		Model::renderFullscreenQuad(commandBuffer, LIGHTING, { descriptors.at(LIGHTING).set });
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
	camera->setExtent(newExtent);

	for (auto pipeline : pipelines)
	{
		pipeline->recreate();
	}
}

void Scene::updateDescriptorSets(DescriptorPool *descriptorPool, RenderPassesMap renderPasses)
{
	// Ssao:

    const auto geometryRenderPass = dynamic_cast<GeometryRenderPass*>(renderPasses.at(GEOMETRY));
	std::vector<TextureImage*> textures{
		geometryRenderPass->getTexture(POSITION).get(),
		geometryRenderPass->getTexture(NORMAL).get(),
		ssaoKernel->getNoiseTexture()
	};
	descriptorPool->updateDescriptorSet(
		descriptors.at(SSAO).set,
		{ ssaoKernel->getBuffer(), camera->getSpaceBuffer() },
		textures);

	// Ssao blur:

	descriptorPool->updateDescriptorSet(
		descriptors.at(SSAO_BLUR).set,
		{},
		{ dynamic_cast<SsaoRenderPass*>(renderPasses.at(SSAO))->getSsaoTexture().get() });

	// Lighting:

	const auto ssaoTexture = dynamic_cast<SsaoRenderPass*>(renderPasses.at(SSAO_BLUR))->getSsaoTexture().get();
    const auto shadowsTexture = dynamic_cast<DepthRenderPass*>(renderPasses.at(DEPTH))->getDepthTexture().get();
	textures = geometryRenderPass->getGBuffer();
	textures.push_back(ssaoTexture);
	textures.push_back(shadowsTexture);
	descriptorPool->updateDescriptorSet(
		descriptors.at(LIGHTING).set,
		{ lighting->getAttributesBuffer(), camera->getSpaceBuffer(), pssmKernel->getSplitsBuffer(), pssmKernel->getSpacesBuffer() },
		textures);
}

// private:

void Scene::initDescriptorSets(DescriptorPool *descriptorPool, RenderPassesMap renderPasses)
{
	DescriptorStruct descriptorStruct{};

    // Depth:

	descriptorStruct.layout = descriptorPool->createDescriptorSetLayout({ VK_SHADER_STAGE_VERTEX_BIT }, {});
	descriptorStruct.set = descriptorPool->getDescriptorSet(descriptorStruct.layout);
	descriptorPool->updateDescriptorSet(descriptorStruct.set, { pssmKernel->getSpacesBuffer() }, {});
	descriptors.insert({ DEPTH, descriptorStruct });

    // Geometry:

	descriptorStruct.layout = descriptorPool->createDescriptorSetLayout({ VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT }, {});
	descriptorStruct.set = descriptorPool->getDescriptorSet(descriptorStruct.layout);
	descriptorPool->updateDescriptorSet(descriptorStruct.set, { camera->getSpaceBuffer(), lighting->getAttributesBuffer() }, {});
	descriptors.insert({ GEOMETRY, descriptorStruct });

    // Ssao:

    const auto geometryRenderPass = dynamic_cast<GeometryRenderPass*>(renderPasses.at(GEOMETRY));
	std::vector<TextureImage*> textures{
		geometryRenderPass->getTexture(POSITION).get(),
		geometryRenderPass->getTexture(NORMAL).get(),
		ssaoKernel->getNoiseTexture()
	};

	std::vector<VkShaderStageFlags> texturesShaderStages(textures.size(), VK_SHADER_STAGE_FRAGMENT_BIT);

	descriptorStruct.layout = descriptorPool->createDescriptorSetLayout(
	    {VK_SHADER_STAGE_FRAGMENT_BIT, VK_SHADER_STAGE_FRAGMENT_BIT},
	    texturesShaderStages);
	descriptorStruct.set = descriptorPool->getDescriptorSet(descriptorStruct.layout);
	descriptorPool->updateDescriptorSet(
		descriptorStruct.set,
		{ ssaoKernel->getBuffer(), camera->getSpaceBuffer() },
		textures);
	descriptors.insert({ SSAO, descriptorStruct });

    // Ssao blur:

	descriptorStruct.layout = descriptorPool->createDescriptorSetLayout({}, { VK_SHADER_STAGE_FRAGMENT_BIT });
	descriptorStruct.set = descriptorPool->getDescriptorSet(descriptorStruct.layout);
	descriptorPool->updateDescriptorSet(
		descriptorStruct.set,
		{},
		{ dynamic_cast<SsaoRenderPass*>(renderPasses.at(SSAO))->getSsaoTexture().get() });
	descriptors.insert({ SSAO_BLUR, descriptorStruct });

    // Lighting:

	const auto ssaoTexture = dynamic_cast<SsaoRenderPass*>(renderPasses.at(SSAO_BLUR))->getSsaoTexture().get();
    const auto shadowsTexture = dynamic_cast<DepthRenderPass*>(renderPasses.at(DEPTH))->getDepthTexture().get();
	textures = geometryRenderPass->getGBuffer();
	textures.push_back(ssaoTexture);
	textures.push_back(shadowsTexture);
	texturesShaderStages = std::vector<VkShaderStageFlags>(textures.size(), VK_SHADER_STAGE_FRAGMENT_BIT);

	descriptorStruct.layout = descriptorPool->createDescriptorSetLayout(
		{ VK_SHADER_STAGE_FRAGMENT_BIT, VK_SHADER_STAGE_FRAGMENT_BIT, VK_SHADER_STAGE_FRAGMENT_BIT, VK_SHADER_STAGE_FRAGMENT_BIT },
		texturesShaderStages);
	descriptorStruct.set = descriptorPool->getDescriptorSet(descriptorStruct.layout);
	descriptorPool->updateDescriptorSet(
		descriptorStruct.set,
		{ lighting->getAttributesBuffer(), camera->getSpaceBuffer(), pssmKernel->getSplitsBuffer(), pssmKernel->getSpacesBuffer() },
		textures);
	descriptors.insert({ LIGHTING, descriptorStruct });

    // Final:

	descriptorStruct.layout = descriptorPool->createDescriptorSetLayout(
		{ VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT, VK_SHADER_STAGE_FRAGMENT_BIT, VK_SHADER_STAGE_FRAGMENT_BIT },
		{ VK_SHADER_STAGE_FRAGMENT_BIT });
	descriptorStruct.set = descriptorPool->getDescriptorSet(descriptorStruct.layout);
	descriptorPool->updateDescriptorSet(
		descriptorStruct.set,
		{ camera->getSpaceBuffer(), lighting->getAttributesBuffer(), pssmKernel->getSplitsBuffer(), pssmKernel->getSpacesBuffer() },
		{ shadowsTexture });
	descriptors.insert({ FINAL, descriptorStruct });

	skybox->initDescriptorSets(descriptorPool);
	terrain->initDescriptorSets(descriptorPool);
	for (const auto &[key, model] : models)
	{
		model->initDescriptorSets(descriptorPool);
	}
}

void Scene::initPipelines(RenderPassesMap renderPasses)
{
	const std::string skyboxShadersDir = "Shaders/Skybox/";

    std::unordered_map<RenderPassType, std::string> shadersDirectories{
		{ DEPTH, "Shaders/Depth" },
		{ GEOMETRY, "Shaders/Geometry" },
		{ FINAL, "Shaders/Final" }
	};

    std::vector<std::shared_ptr<ShaderModule>> shaderModules{
		std::make_shared<ShaderModule>(device, File::getPath(skyboxShadersDir, "Vert.spv"), VK_SHADER_STAGE_VERTEX_BIT),
		std::make_shared<ShaderModule>(device, File::getPath(skyboxShadersDir, "Frag.spv"), VK_SHADER_STAGE_FRAGMENT_BIT)
	};

	pipelines.push_back(skybox->createPipeline(
        FINAL,
        renderPasses.at(FINAL),
        { descriptors.at(FINAL).layout },
		{},
        shaderModules));

    for (const auto &[type, directory] : shadersDirectories)
    {
		shaderModules = std::vector<std::shared_ptr<ShaderModule>>{
			std::make_shared<ShaderModule>(device, File::getPath(directory, "Vert.spv"), VK_SHADER_STAGE_VERTEX_BIT)
		};

		std::vector<VkSpecializationMapEntry> constantEntries;
		std::vector<const void*> constantData;
		std::vector<VkPushConstantRange> pushConstantRanges;

        if (type == DEPTH)
        {
            pushConstantRanges = {
                { VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(uint32_t) }
			};
        }
        if (type == FINAL)
        {
			constantEntries = {
				{ 0, 0, sizeof(uint32_t) },
				{ 1, sizeof(uint32_t), sizeof(float) }
			};
			constantData = { &PssmKernel::CASCADE_COUNT, &pssmKernel->BIAS };
        }

		shaderModules.push_back(
			std::make_shared<ShaderModule>(
				device,
				File::getPath(directory, "Frag.spv"),
				VK_SHADER_STAGE_FRAGMENT_BIT,
				constantEntries,
				constantData));

		pipelines.push_back(terrain->createPipeline(
            type,
            renderPasses.at(type),
            { descriptors.at(type).layout },
			pushConstantRanges,
            shaderModules));

        for (const auto &[key, model] : models)
        {
        	model->setPipeline(type, terrain->getPipeline(type));
        }
    }
}

void Scene::initStaticPipelines(const RenderPassesMap &renderPasses)
{
    const auto fullscreenVertexShader = std::make_shared<ShaderModule>(
        device,
		"Shaders/Fullscreen/Vert.spv",
        VK_SHADER_STAGE_VERTEX_BIT);

    #pragma region Ssao

	std::vector<VkSpecializationMapEntry> ssaoConstantEntries{
		{ 0, 0, sizeof(uint32_t)},
		{ 1, sizeof(uint32_t), sizeof(float)},
		{ 2, sizeof(uint32_t) + sizeof(float), sizeof(float)}
	};
	std::vector<const void*> data = { &ssaoKernel->SIZE, &ssaoKernel->RADIUS, &ssaoKernel->POWER };
    const auto ssaoFragmentShader = std::make_shared<ShaderModule>(
		device,
		"Shaders/Ssao/Frag.spv",
		VK_SHADER_STAGE_FRAGMENT_BIT,
		ssaoConstantEntries,
		data);
    const auto ssaoPipeline = new GraphicsPipeline(
		device,
		renderPasses.at(SSAO),
		{ descriptors.at(SSAO).layout },
		{},
		{ fullscreenVertexShader, ssaoFragmentShader },
		{},
		{},
		false);

	Model::setStaticPipeline(SSAO, ssaoPipeline);
	pipelines.push_back(ssaoPipeline);

    #pragma endregion 

    #pragma region SsaoBlur

    const VkSpecializationMapEntry ssaoBlurConstantEntry{
		0,    
		0,              
		sizeof(uint32_t)
	};

	const auto ssaoBlurFragmentShader = std::make_shared<ShaderModule>(
		device,
		"Shaders/SsaoBlur/Frag.spv",
		VK_SHADER_STAGE_FRAGMENT_BIT,
		std::vector<VkSpecializationMapEntry>{ ssaoBlurConstantEntry },
		std::vector<const void*>{ &ssaoKernel->BLUR_RADIUS });
    const auto ssaoBlurPipeline = new GraphicsPipeline(
		device,
		renderPasses.at(SSAO_BLUR),
		{ descriptors.at(SSAO_BLUR).layout },
		{},
		{ fullscreenVertexShader, ssaoBlurFragmentShader },
		{},
		{},
		false);

	Model::setStaticPipeline(SSAO_BLUR, ssaoBlurPipeline);
	pipelines.push_back(ssaoBlurPipeline);

    #pragma endregion 

    #pragma region Lighting

	std::vector<VkSpecializationMapEntry> lightingConstantEntries{
		{ 0, 0, sizeof(uint32_t)},
		{ 1, sizeof(uint32_t), sizeof(uint32_t)},
		{ 1, sizeof(uint32_t) * 2, sizeof(float)},
	};

	const uint32_t sampleCount = device->getSampleCount();
    const auto lightingFragmentShader = std::make_shared<ShaderModule>(
        device,
		"Shaders/Lighting/Frag.spv",
        VK_SHADER_STAGE_FRAGMENT_BIT, 
		lightingConstantEntries,
        std::vector<const void*>{ &sampleCount, &PssmKernel::CASCADE_COUNT, &pssmKernel->BIAS });

    const auto lightingPipeline = new GraphicsPipeline(
		device,
		renderPasses.at(LIGHTING),
		{ descriptors.at(LIGHTING).layout },
		{},
		{ fullscreenVertexShader, lightingFragmentShader },
		{},
		{},
		false);

	Model::setStaticPipeline(LIGHTING, lightingPipeline);
	pipelines.push_back(lightingPipeline);

    #pragma endregion
}
