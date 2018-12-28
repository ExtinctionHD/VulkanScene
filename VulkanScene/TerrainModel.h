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
	VkVertexInputBindingDescription getVertexInputBindingDescription(uint32_t inputBinding) override;

	std::vector<VkVertexInputAttributeDescription> getVertexInputAttributeDescriptions(uint32_t inputBinding) override;

private:
	glm::vec2 cellSize{};

	// number of cells per side
	VkExtent2D cellCount{};

	std::vector<TextureImage*> textures;

	void initMaterial(const std::string &texturesDirectory, const std::string &extension);

	void initMesh();
};

