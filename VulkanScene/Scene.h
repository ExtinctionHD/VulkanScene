#pragma once

#include "Camera.h"
#include "Timer.h"
#include "Lighting.h"
#include "AssimpModel.h"
#include "SkyboxModel.h"
#include "File.h"
#include "TerrainModel.h"
#include "SsaoKernel.h"
#include "SceneDao.h"

class Scene
{
public:
	Scene(Device *pDevice, VkExtent2D cameraExtent, const std::string &path, float shadowsDistance);
	~Scene();

	uint32_t getBufferCount() const;

	uint32_t getTextureCount() const;

	uint32_t getDescriptorSetCount() const;

	Camera *getCamera() const;

	void prepareSceneRendering(DescriptorPool *pDescriptorPool, const RenderPassesMap &renderPasses);

	void updateScene();

	void render(VkCommandBuffer commandBuffer, RenderPassType type);

	void resizeExtent(VkExtent2D newExtent);

	void updateDescriptorSets(DescriptorPool *pDescriptorPool, RenderPassesMap renderPasses);

private:
	Device *pDevice;

	SceneDao sceneDao;

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
	SkyboxModel *skybox{};
	TerrainModel *terrain{};
	std::unordered_map<std::string, AssimpModel*> models;

	std::vector<GraphicsPipeline*> pipelines;

	void initDescriptorSets(DescriptorPool *pDescriptorPool, RenderPassesMap renderPasses);

	void initPipelines(RenderPassesMap renderPasses);

	void initStaticPipelines(RenderPassesMap renderPasses);
};

