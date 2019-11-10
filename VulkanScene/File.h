#pragma once

#include <vector>
#include <string>

class File
{
public:
	static std::vector<char> getBytes(const std::string &path);

	static std::string getBaseDirectory();

	static std::string getAbsolute(const std::string &path);

	static std::string getDirectory(const std::string &path);

	static std::string getFilename(const std::string &path);

	static std::string getPath(const std::string &directory, const std::string &path);

	static bool exists(const std::string &path);
};

