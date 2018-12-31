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
	VkVertexInputBindingDescription  getVertexBindingDescription(uint32_t binding) override;

	std::vector<VkVertexInputAttributeDescription> getVertexAttributeDescriptions(uint32_t binding, uint32_t locationOffset) override;

private:


	TextureImage *pTexture;
};

