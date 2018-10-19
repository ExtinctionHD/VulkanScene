#pragma once

#include "Mesh.h"
#include "Position.h"
#include <array>
#include "Model.h"

// cube mesh with cube texture
class SkyboxModel : public Model
{
public:
	static const int CUBE_SIDE_COUNT = 6;

	SkyboxModel(Device *pDevice, std::string texturesDir, std::string extension);
	~SkyboxModel();

protected:
	virtual VkVertexInputBindingDescription  getVertexInputBindingDescription(uint32_t inputBinding) override;

	virtual std::vector<VkVertexInputAttributeDescription> getVertexInputAttributeDescriptions(uint32_t inputBinding) override;

private:
	TextureImage *pTexture;
};

