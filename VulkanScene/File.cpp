#include <fstream>
#include <cassert>
#include <string>
#include <filesystem>
#include <Windows.h>

#include "File.h"

// public:

std::vector<char> File::getBytes(const std::string &path)
{
	std::ifstream file(getAbsolute(path), std::ios::ate | std::ios::binary);

	assert(file.is_open());

    const size_t fileSize = size_t(file.tellg());
	std::vector<char> stagingBuffer(fileSize);

	file.seekg(0);
	file.read(stagingBuffer.data(), fileSize);

	file.close();

	return stagingBuffer;
}

std::string File::getBaseDirectory()
{
	char stagingBuffer[MAX_PATH];
	GetModuleFileName(nullptr, stagingBuffer, MAX_PATH);
    const std::string path(stagingBuffer);

	return std::filesystem::path(path).parent_path().parent_path().string();
}

std::string File::getAbsolute(const std::string &path)
{
	return getBaseDirectory() + "\\" + path;
}

std::string File::getDirectory(const std::string &path)
{
	return std::filesystem::path(path).remove_filename().string();
}

std::string File::getFilename(const std::string &path)
{
	return std::filesystem::path(path).filename().string();
}

std::string File::getPath(const std::string &directory, const std::string &path)
{
	return std::filesystem::path(directory).append(path).string();
}

bool File::exists(const std::string &path)
{
	std::ifstream f(getAbsolute(path).c_str());
	return f.good();
}
