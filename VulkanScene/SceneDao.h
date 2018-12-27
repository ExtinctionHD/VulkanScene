#pragma once

#include <string>
#include <nlohmann/json.hpp>


class SceneDao
{
public:
	SceneDao(const std::string& filename);
	~SceneDao() = default;

	static void saveScene(const std::string& filename);

private:
	nlohmann::json scene;
};

