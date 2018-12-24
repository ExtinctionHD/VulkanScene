#include <fstream>
#include <Windows.h>
#include <algorithm>
#include <cassert>
#include <string>
#include <sstream>

#include "File.h"

// public:

std::vector<char> File::getFileBytes(const std::string &filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	assert(file.is_open());

	size_t fileSize = size_t(file.tellg());
	std::vector<char> stagingBuffer(fileSize);

	file.seekg(0);
	file.read(stagingBuffer.data(), fileSize);

	file.close();

	return stagingBuffer;
}

std::string File::getExeDir()
{
	char stagingBuffer[MAX_PATH];
	GetModuleFileName(nullptr, stagingBuffer, MAX_PATH);
	std::string path(stagingBuffer);

	std::replace(path.begin(), path.end(), '\\', '/');

	return getFileDir(path);
}

std::string File::getFileDir(const std::string &path)
{
	return path.substr(0, path.find_last_of('/') + 1);
}

std::string File::getFilename(std::string path)
{
	std::replace(path.begin(), path.end(), '\\', '/');

	return path.substr(path.find_last_of('/') + 1);
}

bool File::exists(const std::string &filename)
{
	std::ifstream f(filename.c_str());
	return f.good();
}

Lighting::Attributes File::getLightingAttributes(const std::string &filename, std::string &skyboxDir, std::string &skyboxExtension)
{
    std::ifstream source;
	source.open(filename, std::ios_base::in);

	assert(std::getline(source, skyboxDir));
	skyboxDir = getExeDir() + skyboxDir;
	assert(std::getline(source, skyboxExtension));

	Lighting::Attributes attributes{};
	std::string line;

	assert(std::getline(source, line));
	std::istringstream input(line);
	float r;
	input >> r;
	float g;
	input >> g;
	float b;
	input >> b;
	attributes.color = glm::vec3(r, g, b);

	assert(std::getline(source, line));
	input = std::istringstream(line);
	input >> attributes.ambientStrength;

	assert(std::getline(source, line));
	input = std::istringstream(line);
	float x;
	input >> x;
	float y;
	input >> y;
	float z;
	input >> z;
	attributes.direction = glm::vec3(x, y, z);

	assert(std::getline(source, line));
	input = std::istringstream(line);
	input >> attributes.directedStrength;

	assert(std::getline(source, line));
	input = std::istringstream(line);
	input >> attributes.specularPower;

	return attributes;
}
