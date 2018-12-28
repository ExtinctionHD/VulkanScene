#pragma once

#include <vector>
#include "Lighting.h"

class File
{
public:
	// returns bytes of file
	static std::vector<char> getBytes(const std::string &path);

	// returns directory of application executable file
	static std::string getExeDirectory();

	static std::string getAbsolute(const std::string& path);

	// returns directory of file
	static std::string getDirectory(const std::string &path);

	// get only filename from full path
	static std::string getFilename(const std::string& path);

	static std::string getPath(const std::string& directory, const std::string& path);

	// returns true if file exists
	static bool exists(const std::string &path);
};

