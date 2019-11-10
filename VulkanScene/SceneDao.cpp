#include <fstream>
#include <glm/glm.hpp>
#include "File.h"

#include "SceneDao.h"

// public:

SceneDao::SceneDao(const std::string &path)
{
	open(path);
}

void SceneDao::open(const std::string &path)
{
	std::ifstream stream(File::getAbsolute(path));
	stream >> scene;
}

Camera::Attributes SceneDao::getCameraAttributes() const
{
	nlohmann::json camera = scene["camera"];

	if (camera.find("external") != camera.end())
	{
		std::ifstream stream(File::getAbsolute(camera["external"]));
		camera = {};
		stream >> camera;
	}

	Camera::Attributes attributes{
		getVec3(camera["position"]),
		getVec3(camera["forward"]),
		getVec3(camera["up"]),
		camera["fov"].get<float>(),
		camera["speed"].get<float>(),
		camera["sensitivity"].get<float>(),
		camera["nearPlane"].get<float>(),
		camera["farPlane"].get<float>(),
	};

	return attributes;
}

Lighting::Attributes SceneDao::getLightingAttributes() const
{
	nlohmann::json lighting = scene["lighting"];

	if (lighting.find("external") != lighting.end())
	{
		std::ifstream stream(File::getAbsolute(lighting["external"]));
		lighting = {};
		stream >> lighting;
	}

	Lighting::Attributes attributes{
		getVec3(lighting["color"]),
		lighting["ambientStrength"].get<float>(),
		getVec3(lighting["direction"]),
		lighting["directedStrength"].get<float>(),
		getCameraAttributes().position,
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

std::unordered_map<std::string, AssimpModel*> SceneDao::getModels(Device *device)
{
	return parseModels(device, scene["models"]);
}

void SceneDao::saveScene(const std::string &path)
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
		{ "sensitivity", 0.1f },
        { "nearPlane", 0.01f },
        { "farPlane", 500.0f}
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

	scene["models"]["Regera"] = {
		{ "path", "models/Koenigsegg Regera/Regera.fbx" }
	};
	scene["models"]["Regera"]["transformations"][0][0] = {
		{ "type", "MOVE" },
		{ "distance", {
			{ "x", 10.0f },
			{ "y", 0.0f },
			{ "z", 1.0f }
		}}
	};
	scene["models"]["Regera"]["transformations"][0][1] = {
		{ "type", "SCALE" },
		{ "size", {
            { "axis", "x" },
            { "value", 2.050f }
		} },
	};
	scene["models"]["Regera"]["transformations"][0][2] = {
		{ "type", "ROTATE" },
		{ "angle", 40 },
		{ "axis", {
			{ "x", 0.0f },
			{ "y", 1.0f },
			{ "z", 0.0f }
		}}
	};
	scene["models"]["Regera"]["transformations"][0][3] = {
		{ "type", "ROTATE" },
		{ "angle", 90 },
		{ "axis", {
			{ "x", 1.0f },
			{ "y", 0.0f },
			{ "z", 0.0f }
		}}
	};

	scene["models"]["House"] = {
		{ "path", "models/House/House.obj" }
	};
	scene["models"]["House"]["transformations"][0][0] = {
		{ "type", "MOVE" },
		{ "distance", {
			{ "x", -2.1f },
			{ "y", 0.14f },
			{ "z", 3.1f }
		}}
	};
	scene["models"]["House"]["transformations"][0][1] = {
		{ "type", "ROTATE" },
		{ "angle", 180 },
		{ "axis", {
			{ "x", 1.0f },
			{ "y", 0.0f },
			{ "z", 0.0f }
		}}
	};

    std::string fullPath = File::getAbsolute(path);
	std::ofstream stream(fullPath);
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

std::unordered_map<std::string, AssimpModel*> SceneDao::parseModels(Device *device, nlohmann::json modelsJson)
{
	std::unordered_map<std::string, AssimpModel*> models;

	for (const auto&[modelName, modelJson] : modelsJson.items())
	{
		if (modelJson.find("external") != modelJson.end())
		{
			std::ifstream stream(File::getAbsolute(modelJson["external"]));
			nlohmann::json externalModelsJson;
			stream >> externalModelsJson;

			std::unordered_map<std::string, AssimpModel*> externalModels = parseModels(device, externalModelsJson);
			models.merge(externalModels);
		}
		else
		{
			nlohmann::json transformationsJson = modelJson["transformations"];
			AssimpModel *model = new AssimpModel(device, modelJson["path"], uint32_t(transformationsJson.size()));

			for (uint32_t i = 0; i < transformationsJson.size(); i++)
			{
				model->setTransformation(getTransformation(transformationsJson[i], model), i);
			}

			models.insert({ modelName, model });
		}

	}

	return models;
}

Transformation SceneDao::getTransformation(nlohmann::json json, AssimpModel *model)
{
	Transformation transformation{ glm::mat4(1.0f) };

	for (const auto& transformationJson : json)
	{
		std::string type = transformationJson["type"];

		if (type == "MOVE")
		{
			transformation.move(getVec3(transformationJson["distance"]));
		}
		else if (type == "SCALE")
		{
			if (transformationJson.find("size") != transformationJson.end())
			{
                const auto size = transformationJson["size"]["value"].get<float>();

				if (transformationJson["size"]["axis"] == "x")
				{
					transformation.scale(glm::vec3(size / model->getBaseSize().x));
				}
				else if (transformationJson["size"]["axis"] == "y")
				{
					transformation.scale(glm::vec3(size / model->getBaseSize().y));
				}
				else if (transformationJson["size"]["axis"] == "z")
				{
					transformation.scale(glm::vec3(size / model->getBaseSize().z));
				}
			}
			else if (transformationJson.find("scale") != transformationJson.end())
			{
				transformation.scale(getVec3(transformationJson["scale"]));
			}
			else
			{
				throw std::invalid_argument("Invalid scale object in transformation");
			}
		}
		else if (type == "ROTATE")
		{
			transformation.rotate(
				getVec3(transformationJson["axis"]),
				transformationJson["angle"].get<float>());
		}
		else
		{
			throw std::invalid_argument("Invalid transformation type");
		}
	}

	return transformation;
}
