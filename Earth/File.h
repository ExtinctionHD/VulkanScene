#pragma once

#include <vector>

class File
{
public:
	// returns bytes of file
	static std::vector<char> getFileBytes(std::string filename);

	// returns directory of application executable file
	static std::string getExeDir();

	// returns directory of file
	static std::string getFileDir(std::string path);

	// get only filename from full path
	static std::string getFilename(std::string path);   
};

