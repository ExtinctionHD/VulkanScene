#pragma once

#include "AssimpMesh.h"
#include "Position.h"
#include <array>
#include "Model.h"

class SkyboxModel : public Model
{
public:
	static const int CUBE_SIDE_COUNT = 6;

	SkyboxModel(Device *pDevice, std::array<std::string, CUBE_SIDE_COUNT> textureFilenames);
	~SkyboxModel();

private:
	AssimpMesh<Position> *pMesh;

	TextureImage *pTexture;
};

