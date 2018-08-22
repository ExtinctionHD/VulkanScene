#include <fstream>
#include "Logger.h"
#include <Windows.h>
#include <algorithm>

#include "File.h"

// public:

std::vector<char> File::getFileBytes(std::string filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open())
	{
		LOGGER_FATAL(Logger::getFileOpeningErrMsg(filename));
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}

std::string File::getExeDir()
{
	char buffer[MAX_PATH];
	GetModuleFileName(NULL, buffer, MAX_PATH);
	std::string path(buffer);

	std::replace(path.begin(), path.end(), '\\', '/');

	return path.substr(0, path.find_last_of("/") + 1);
}
