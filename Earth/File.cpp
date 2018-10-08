#include <fstream>
#include <Windows.h>
#include <algorithm>

#include "File.h"

// public:

std::vector<char> File::getFileBytes(std::string filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open())
	{
		throw std::runtime_error("Failed to open file: " + filename);
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> stagingBuffer(fileSize);

	file.seekg(0);
	file.read(stagingBuffer.data(), fileSize);

	file.close();

	return stagingBuffer;
}

std::string File::getExeDir()
{
	char stagingBuffer[MAX_PATH];
	GetModuleFileName(NULL, stagingBuffer, MAX_PATH);
	std::string path(stagingBuffer);

	std::replace(path.begin(), path.end(), '\\', '/');

	return getFileDir(path);
}

std::string File::getFileDir(std::string path)
{
	return path.substr(0, path.find_last_of("/") + 1);
}

std::string File::getFilename(std::string path)
{
	std::replace(path.begin(), path.end(), '\\', '/');

	return path.substr(path.find_last_of('/') + 1);
}
