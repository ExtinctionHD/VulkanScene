#pragma once

#include <string>
#include <nlohmann/json.hpp>
#include "Lighting.h"
#include "ImageSetInfo.h"
#include "Camera.h"


class SceneDao
{
public:
	SceneDao() = default;
	SceneDao(const std::string& path);

	void open(const std::string& path);

	Camera::Attributes getCameraAttributes() const;

	Lighting::Attributes getLightingAttributes() const;

	ImageSetInfo getSkyboxInfo() const;

	ImageSetInfo getTerrainInfo() const;

	static void saveScene(const std::string& path);

private:
	nlohmann::json scene;

	static ImageSetInfo getImageSetInfo(nlohmann::json json);

	static glm::vec3 getVec3(nlohmann::json json);
};

