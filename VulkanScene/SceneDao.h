#pragma once

#include <string>
#include <nlohmann/json.hpp>
#include "Lighting.h"
#include "ImageSetInfo.h"


class SceneDao
{
public:
	SceneDao() = default;
	SceneDao(const std::string& path);

	void open(const std::string& path);

	Lighting::Attributes getLightingAttributes(glm::vec3 cameraPos);

	ImageSetInfo getSkyboxInfo();

	ImageSetInfo getTerrainInfo();

	static void saveScene(const std::string& path);

private:
	nlohmann::json scene;

	static ImageSetInfo getImageSetInfo(nlohmann::json json);

	static glm::vec3 getVec3(nlohmann::json json);
};

