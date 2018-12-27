#include <fstream>
#include <glm/glm.hpp>

#include "SceneDao.h"

SceneDao::SceneDao(const std::string& filename)
{
	std::ifstream stream(filename);
	stream >> scene;
}

void SceneDao::saveScene(const std::string& filename)
{
	nlohmann::json scene;

	scene["skybox"] = {
		{ "dir", "textures/skyboxNoon/" },
		{ "extension", ".jpg" }
	};
	scene["terrain"] = {
		{ "dir", "textures/grass/" },
		{ "extension", ".jpg" }
	};

	scene["lighting"]["color"] = {
		{ "r", 1.0f },
		{ "g", 1.0f },
		{ "b", 1.0f },
	};
	scene["lighting"]["direction"] = {
		{ "x", 0.0f },
		{ "y", -0.001f },
		{ "z", 1.0f },
	};
	scene["lighting"]["ambientStrength"] = 0.9f;
	scene["lighting"]["directedStrength"] = 1.0f;
	scene["lighting"]["specularPower"] = 16.0f;

	std::ofstream stream(filename);
	stream << scene;
}
