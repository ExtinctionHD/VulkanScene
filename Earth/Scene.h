#pragma once

#include "Camera.h"
#include "Timer.h"
#include "Lighting.h"
#include "AssimpModel.h"
#include "SkyboxModel.h"
#include "File.h"

class Scene
{
public:
	Scene(Device *pDevice, VkExtent2D cameraExtent);
	~Scene();

	void updateScene();

	void draw();

private:
	const std::string MODEL_FILE = File::getExeDir() + "models/mustangGT/mustang_GT.obj";
	const std::string SKYBOX_DIR = File::getExeDir() + "textures/skybox/";

	Device *pDevice;

	// camera attributes
	Camera * pCamera;

	// timer for animations
	Timer frameTimer;

	// scene lighting attributes
	Lighting lighting;

	// models
	AssimpModel *pModel;
	SkyboxModel *pSkybox;
};

