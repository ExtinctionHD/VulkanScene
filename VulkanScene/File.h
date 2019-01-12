#pragma once

#include <vector>

class File
{
public:
	// returns bytes of file
	static std::vector<char> getBytes(const std::string &path);

	// returns base directory
	static std::string getBaseDirectory();

	// returns absolute path from relative to base directory
	static std::string getAbsolute(const std::string& path);

	// returns directory of file
	static std::string getDirectory(const std::string &path);

	// returns only filename from full path
	static std::string getFilename(const std::string& path);

	// returns path relative to directory
	static std::string getPath(const std::string& directory, const std::string& path);

	// returns true if file exists
	static bool exists(const std::string &path);
};

