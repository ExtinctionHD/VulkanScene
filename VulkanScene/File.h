#pragma once

#include <vector>

class File
{
public:
	// returns bytes of file
	static std::vector<char> getFileBytes(const std::string &filename);

	// returns directory of application executable file
	static std::string getExeDir();

	// returns directory of file
	static std::string getFileDir(const std::string &path);

	// get only filename from full path
	static std::string getFilename(std::string path);  

	// returns true if file exists
	static bool exists(const std::string &path);
};

