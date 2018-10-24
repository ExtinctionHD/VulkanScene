#pragma once

#include "Camera.h"
#include "Timer.h"
#include "Lighting.h"
#include "AssimpModel.h"
#include "SkyboxModel.h"
#include "File.h"
#include "Controller.h"
#include "TerrainModel.h"


// provides scene for rendering
// contains: camera, lighting, models
class Scene
{
public:
	Scene(Device *pDevice, VkExtent2D cameraExtent);
	~Scene();

	Controller* getController() const;

	uint32_t getBufferCount() const;;

	uint32_t getTextureCount() const;;

	uint32_t getDescriptorSetCount() const;;

	void initDescriptorSets(DescriptorPool *pDescriptorPool);

	void initPipelines(RenderPass *pRenderPass);

	void updateScene();

	void draw(VkCommandBuffer commandBuffer);

	void resizeExtent(VkExtent2D newExtent);

private:
	Device *pDevice;

	Controller *pController;

	// camera attributes
	Camera *pCamera;

	// timer for animations
	Timer frameTimer;

	// scene lighting attributes
	Lighting lighting;
	Buffer *pLightingBuffer;

	// scene
	VkDescriptorSet sceneDescriptorSet;
	VkDescriptorSetLayout sceneDSLayout;

	// models
	AssimpModel *pCar;
	SkyboxModel *pSkybox;
	TerrainModel *pTerrain;
	std::vector<Model*> models;

	std::vector<GraphicsPipeline*> pipelines;

	void initCamera(VkExtent2D cameraExtent);

	void initLighting();

	void initModels();
};

