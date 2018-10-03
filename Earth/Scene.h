#pragma once

#include "Camera.h"
#include "Timer.h"
#include "Lighting.h"
#include "AssimpModel.h"
#include "SkyboxModel.h"

class Scene
{
public:
	Scene();
	~Scene();

private:
	// camera attributes
	Camera * pCamera;

	// timer for animations
	Timer frameTimer;

	// scene lighting attributes
	Lighting lighting;

	std::vector<Model*> models;
};

