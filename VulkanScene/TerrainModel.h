#pragma once
#include "Model.h"

class TerrainModel : public Model
{
public:
	TerrainModel(Device *pDevice, VkExtent2D size, VkExtent2D cellCount, std::string texturesDir, std::string extension);
	~TerrainModel();

protected:
	virtual VkVertexInputBindingDescription getVertexInputBindingDescription(uint32_t inputBinding) override;

	virtual std::vector<VkVertexInputAttributeDescription> getVertexInputAttributeDescriptions(uint32_t inputBinding) override;

private:
	VkExtent2D size;

	// number of cells per side
	VkExtent2D cellCount;

	std::vector<TextureImage*> textures;

	void initMaterial(std::string texturesDir, std::string extension);

	void initMesh();
};

