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

	// creates a pipeline for rendering models of this class
	static void createPipeline(Device *pDevice, std::vector<VkDescriptorSetLayout> layouts, RenderPass *pRenderPass);

	static void recreatePipeline(RenderPass *pRenderPass);

	static void destroyPipeline();

protected:
	virtual VkDescriptorSetLayout& getMeshDSLayout() override;

	virtual GraphicsPipeline * getPipeline() override;

private:
	enum ShaderTypes { vert, frag };
	static const std::vector<std::string> SHADER_FILES;

	static uint32_t objectCount;

	static VkDescriptorSetLayout meshDSLayout;

	static GraphicsPipeline *pPipeline;

	Material *pMaterial;

	TextureImage *pTexture;
};

