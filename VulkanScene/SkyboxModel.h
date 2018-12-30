#pragma once

#include "Mesh.h"
#include "Position.h"
#include <array>
#include "Model.h"
#include "ImageSetInfo.h"

// cube mesh with cube texture
class SkyboxModel : public Model
{
public:
	static const int CUBE_SIDE_COUNT = 6;

	static const std::vector<std::string> FILENAMES;

	SkyboxModel(Device *pDevice, ImageSetInfo imageSetInfo);
	~SkyboxModel();

protected:
	VkVertexInputBindingDescription  getVertexInputBindingDescription(uint32_t inputBinding) override;

	std::vector<VkVertexInputAttributeDescription> getVertexInputAttributeDescriptions(uint32_t inputBinding) override;

private:


	TextureImage *pTexture;
};

