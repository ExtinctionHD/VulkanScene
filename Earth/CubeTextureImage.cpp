#include "CubeTextureImage.h"

CubeTextureImage::CubeTextureImage(Device *pDevice, std::array<std::string, CUBE_SIDE_COUNT> filenames) :
	TextureImage(pDevice, std::vector<std::string>(filenames.begin(), filenames.end()), CUBE_SIDE_COUNT, true)
{
}

CubeTextureImage::~CubeTextureImage()
{
}
