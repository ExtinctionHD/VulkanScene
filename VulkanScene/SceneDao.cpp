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

Camera::Attributes SceneDao::getCameraAttributes() const
{
	nlohmann::json camera = scene["camera"];

	Camera::Attributes attributes{
		getVec3(camera["position"]),
		getVec3(camera["forward"]),
		getVec3(camera["up"]),
		camera["fov"].get<float>(),
		camera["speed"].get<float>(),
		camera["sensitivity"].get<float>(),
	};

	return attributes;
}

Lighting::Attributes SceneDao::getLightingAttributes() const
{
	nlohmann::json lighting = scene["lighting"];

	Lighting::Attributes attributes{
		getVec3(lighting["color"]),
		lighting["ambientStrength"].get<float>(),
		getVec3(lighting["direction"]),
		lighting["directedStrength"].get<float>(),
		getVec3(scene["camera"]["position"]),
		lighting["specularPower"].get<float>()
	};

	return attributes;
}

ImageSetInfo SceneDao::getSkyboxInfo() const
{
	return getImageSetInfo(scene["skybox"]);
}

ImageSetInfo SceneDao::getTerrainInfo() const
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
		{ "b", 1.0f }
	};
	scene["lighting"]["direction"] = {
		{ "x", 0.0f },
		{ "y", 1.0f },
		{ "z", -0.001f }
	};
	scene["lighting"]["ambientStrength"] = 0.9f;
	scene["lighting"]["directedStrength"] = 1.0f;
	scene["lighting"]["specularPower"] = 16.0f;

	scene["camera"] = {
		{ "fov", 45.0f },
		{ "speed", 2.0f },
		{ "sensitivity", 0.1f }
	};
	scene["camera"]["position"] = {
		{ "x", 0.0f },
		{ "y", -3.0f },
		{ "z", -6.0f }
	};
	scene["camera"]["forward"] = {
		{ "x", 0.0f },
		{ "y", -0.8f },
		{ "z", 1.0f }
	};
	scene["camera"]["up"] = {
		{ "x", 0.0f },
		{ "y", -1.0f },
		{ "z", 0.0f }
	};

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
