#pragma once

#include "Camera.h"
#include "Timer.h"
#include "Lighting.h"
#include "AssimpModel.h"
#include "SkyboxModel.h"
#include "File.h"
#include "Controller.h"
#include "TerrainModel.h"
#include "SsaoKernel.h"

// provides scene for rendering
// contains: camera, lighting, models
class Scene
{
public:
	Scene(Device *pDevice, VkExtent2D cameraExtent);
	~Scene();

	Controller* getController() const;

	uint32_t getBufferCount() const;

	uint32_t getTextureCount() const;

	uint32_t getDescriptorSetCount() const;

	void prepareSceneRendering(DescriptorPool *pDescriptorPool, const RenderPassesMap &renderPasses);

	void updateScene();

	void render(VkCommandBuffer commandBuffer, RenderPassType type);

	void resizeExtent(VkExtent2D newExtent);

private:
	Device *pDevice;

	Controller *pController;

	// camera attributes
	Camera *pCamera{};

	// timer for animations
	Timer frameTimer;

	// scene lighting attributes
	Lighting *pLighting{};

	SsaoKernel *pSsaoKernel{};

	// scene descriptors
	std::unordered_map<RenderPassType, DescriptorStruct> descriptors;

	// models
	AssimpModel *pRegera{};
	AssimpModel *pAmgGt{};
	AssimpModel *pVulcan{};
	AssimpModel *pHouse{};
	AssimpModel *pSugarMarple{};
	AssimpModel *pNorwayMarple{};
	SkyboxModel *pSkybox{};
	TerrainModel *pTerrain{};
	std::vector<Model*> models;

	std::vector<GraphicsPipeline*> pipelines;

	void initCamera(VkExtent2D cameraExtent);

	void initLighting();

	void initModels();

	void initDescriptorSets(DescriptorPool *pDescriptorPool, RenderPassesMap renderPasses);

	void initPipelines(RenderPassesMap renderPasses);

	void initStaticPipelines(RenderPassesMap renderPasses);
};

