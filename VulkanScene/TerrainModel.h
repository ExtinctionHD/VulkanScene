#pragma once
#include "Model.h"

class TerrainModel : public Model
{
public:
	TerrainModel(
		Device *pDevice, 
		VkExtent2D size, 
		VkExtent2D cellCount, 
		const std::string &texturesDir, 
		const std::string &extension
	);

	~TerrainModel();

protected:
	VkVertexInputBindingDescription getVertexInputBindingDescription(uint32_t inputBinding) override;

	std::vector<VkVertexInputAttributeDescription> getVertexInputAttributeDescriptions(uint32_t inputBinding) override;

private:
	VkExtent2D size{};

	// number of cells per side
	VkExtent2D cellCount{};

	std::vector<TextureImage*> textures;

	void initMaterial(const std::string &texturesDir, const std::string &extension);

	void initMesh();
};

