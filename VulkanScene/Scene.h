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

	// creates pipelines for each model class
	void createPipelines(RenderPass *pRenderPass);

	void updateScene();

	void draw(VkCommandBuffer commandBuffer);

	void resizeExtent(RenderPass *pRenderPass);

private:
	const std::string AVENTADOR_FILE = File::getExeDir() + "models/aventador/lamborghini_aventador.fbx";
	const std::string MUSTANG_FILE = File::getExeDir() + "models/mustangGT/mustang_GT.obj";
	const std::string FORD_FILE = File::getExeDir() + "models/fordGT/Ford GT 2017.obj";

	const std::string SKYBOX_DIR = File::getExeDir() + "textures/skybox1/";

	Device *pDevice;

	Controller *pController;

	// camera attributes
	Camera *pCamera;

	// timer for animations
	Timer frameTimer;

	// scene lighting attributes
	Lighting lighting;
	Buffer *pLightingBuffer;

	ViewProjMatrices viewProj;
	Buffer *pViewProjBuffer;

	VkDescriptorSet sceneDescriptorSet;
	VkDescriptorSetLayout sceneDSLayout;

	// models
	AssimpModel *pCar;
	SkyboxModel *pSkybox;
	std::vector<Model*> models;

	void initCamera(VkExtent2D cameraExtent);

	void initLighting();

	void initModels();
};

