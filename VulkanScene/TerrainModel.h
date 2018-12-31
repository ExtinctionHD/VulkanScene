#pragma once

#include "Model.h"
#include "ImageSetInfo.h"

class TerrainModel : public Model
{
public:
	TerrainModel(
		Device *pDevice, 
		glm::vec2 cellSize,
		VkExtent2D cellCount,
		ImageSetInfo imageSetInfo
	);

	~TerrainModel();

protected:
	VkVertexInputBindingDescription getVertexBindingDescription(uint32_t binding) override;

	std::vector<VkVertexInputAttributeDescription> getVertexAttributeDescriptions(uint32_t binding, uint32_t locationOffset) override;

private:
	glm::vec2 cellSize{};

	// number of cells per side
	VkExtent2D cellCount{};

	std::vector<TextureImage*> textures;

	void initMaterial(const std::string &texturesDirectory, const std::string &extension);

	void initMesh();
};

