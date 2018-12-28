#include <fstream>
#include <glm/glm.hpp>
#include "File.h"

#include "SceneDao.h"

// public:

SceneDao::SceneDao(const std::string& path)
{
	open(path);
}

void SceneDao::open(const std::string& path)
{
	std::ifstream stream(File::getAbsolute(path));
	stream >> scene;
}

Lighting::Attributes SceneDao::getLightingAttributes(glm::vec3 cameraPos)
{
	nlohmann::json lighting = scene["lighting"];

	Lighting::Attributes attributes{
		getVec3(lighting["color"]),
		lighting["ambientStrength"].get<float>(),
		getVec3(lighting["direction"]),
		lighting["directedStrength"].get<float>(),
		cameraPos,
		lighting["specularPower"].get<float>()
	};

	return attributes;
}

ImageSetInfo SceneDao::getSkyboxInfo()
{
	return getImageSetInfo(scene["skybox"]);
}

ImageSetInfo SceneDao::getTerrainInfo()
{
	return getImageSetInfo(scene["terrain"]);
}

void SceneDao::saveScene(const std::string& path)
{
	nlohmann::json scene;

	scene["skybox"] = {
		{ "directory", "textures/skyboxNoon" },
		{ "extension", ".jpg" }
	};
	scene["terrain"] = {
		{ "directory", "textures/grass" },
		{ "extension", ".jpg" }
	};

	scene["lighting"]["color"] = {
		{ "r", 1.0f },
		{ "g", 1.0f },
		{ "b", 1.0f },
	};
	scene["lighting"]["direction"] = {
		{ "x", 0.0f },
		{ "y", 1.0f },
		{ "z", -0.001f },
	};
	scene["lighting"]["ambientStrength"] = 0.9f;
	scene["lighting"]["directedStrength"] = 1.0f;
	scene["lighting"]["specularPower"] = 16.0f;

	std::ofstream stream(File::getAbsolute(path));
	stream << scene;
}

// private:

ImageSetInfo SceneDao::getImageSetInfo(nlohmann::json json)
{
	ImageSetInfo imageSetInfo{
		   json["directory"],
		   json["extension"]
	};

	return imageSetInfo;
}

glm::vec3 SceneDao::getVec3(nlohmann::json json)
{
	glm::vec3 vector;

	if (json.find("x") != json.end())
	{
		vector = glm::vec3(json["x"], json["y"], json["z"]);
	}
	else if (json.find("r") != json.end())
	{
		vector = glm::vec3(json["r"], json["g"], json["b"]);
	}
	else
	{
		throw std::invalid_argument("Json object doesn't contain vector");
	}

	return vector;
}
