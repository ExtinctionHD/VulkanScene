#pragma once
#include "TextureImage.h"
#include <array>

class CubeTextureImage : public TextureImage
{
public:
	static const int CUBE_SIDE_COUNT = 6;

	CubeTextureImage(Device *pDevice, std::array<std::string, CUBE_SIDE_COUNT> filenames);
	~CubeTextureImage();

private:
};

