#include "AssimpModel.h"
#include "SkyboxModel.h"

#include "Scene.h"

// public:

Scene::Scene(Device *pDevice, VkExtent2D cameraExtent)
{
	this->pDevice = pDevice;

	pCamera = new Camera(cameraExtent);

	pModel = new AssimpModel(pDevice, MODEL_FILE);
	pSkybox = new SkyboxModel(pDevice, SKYBOX_DIR, ".jpg");
}

Scene::~Scene()
{
	delete(pModel);
	delete(pSkybox);

	delete(pCamera);
}

void Scene::updateScene()
{
	double deltaSec = frameTimer.getDeltaSec();
}

void Scene::draw()
{
}
