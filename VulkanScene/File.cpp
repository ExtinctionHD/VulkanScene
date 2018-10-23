#include <fstream>
#include <Windows.h>
#include <algorithm>
#include <cassert>

#include "File.h"

// public:

std::vector<char> File::getFileBytes(std::string filename)
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
	GetModuleFileName(NULL, stagingBuffer, MAX_PATH);
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
