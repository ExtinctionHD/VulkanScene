#pragma once

#include "Mesh.h"
#include "Position.h"
#include <array>
#include "Model.h"

class SkyboxModel : public Model
{
public:
	static const int CUBE_SIDE_COUNT = 6;

	SkyboxModel(Device *pDevice, std::string texturesDir, std::string extension);
	~SkyboxModel();

protected:
	void virtual initMeshDescriptorSets(DescriptorPool *pDescriptorPool) override;

private:
	static uint32_t objectCount;

	static VkDescriptorSetLayout meshDSLayout;

	Mesh<Position> *pMesh;

	TextureImage *pTexture;
};

