#pragma once

#include "Camera.h"
#include "Timer.h"
#include "Lighting.h"
#include "AssimpModel.h"
#include "SkyboxModel.h"
#include "File.h"
#include "Controller.h"

// provides scene for rendering
// contains: camera, lighting, models
class Scene
{
public:
	Scene(Device *pDevice, VkExtent2D cameraExtent);
	~Scene();

	uint32_t getBufferCount();

	uint32_t getTextureCount();

	uint32_t getDecriptorSetCount();

	void initDescriptorSets(DescriptorPool *pDescriptorPool);

	// creates pipelines for each model class
	void createPipelines(RenderPass *pRenderPass);

	void updateScene();

	void draw(VkCommandBuffer commandBuffer);

	void resizeExtent(RenderPass *pRenderPass);

private:
	const std::string MUSTANG_FILE = File::getExeDir() + "models/aventador/lamborghini_aventador.fbx";
	const std::string SKYBOX_DIR = File::getExeDir() + "textures/skybox1/";

	Device *pDevice;

	Controller controller;

	// camera attributes
	Camera *pCamera;

	// timer for animations
	Timer frameTimer;

	// scene lighting attributes
	Lighting lighting;
	Buffer *pLightingBuffer;

	VkDescriptorSet lightingDescriptorSet;
	VkDescriptorSetLayout lightingDSLayout;

	// models
	AssimpModel *pMustang;
	SkyboxModel *pSkybox;
	std::vector<Model*> models;

	void initCamera(VkExtent2D cameraExtent);

	void initLighting();

	void initModels();
};

