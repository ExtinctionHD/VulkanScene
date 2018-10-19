#pragma once

#include "Camera.h"
#include "Timer.h"
#include "Lighting.h"
#include "AssimpModel.h"
#include "SkyboxModel.h"
#include "File.h"
#include "Controller.h"
#include "ViewProjMatrices.h"

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

	uint32_t getDecriptorSetCount() const;;

	void initDescriptorSets(DescriptorPool *pDescriptorPool);

	void createPipelines(RenderPass *pRenderPass);

	void updateScene();

	void draw(VkCommandBuffer commandBuffer);

	void resizeExtent(RenderPass *pRenderPass);

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
	ViewProjMatrices viewProj;
	Buffer *pViewProjBuffer;

	VkDescriptorSet sceneDescriptorSet;
	VkDescriptorSetLayout sceneDSLayout;

	// models
	AssimpModel *pCar;
	SkyboxModel *pSkybox;
	std::vector<Model*> models;

	std::vector<GraphicsPipeline*> pipelines;

	void initCamera(VkExtent2D cameraExtent);

	void initLighting();

	void initModels();
};

