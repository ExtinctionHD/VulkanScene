#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>
#include "Mesh.h"
#include <vector>
#include <map>
#include "Model.h"

#pragma comment(lib, "assimp-vc140-mt.lib")

// model that loaded from file using Assimp
class AssimpModel : public Model
{
public:
	AssimpModel(Device *pDevice, const std::string& filename);
	~AssimpModel();

	// creates pipeline for rendering models of this class
	static void createPipeline(Device *pDevice, std::vector<VkDescriptorSetLayout> layouts, RenderPass * pRenderPass);

	static void recreatePipeline(RenderPass *pRenderPass);

	static void destroyPipeline();

protected:
	virtual VkDescriptorSetLayout& getMeshDSLayout() override;

	virtual GraphicsPipeline *getPipeline() override;

private:
	enum ShaderTypes { vert, frag };
	static const std::vector<std::string> SHADER_FILES;

	static uint32_t objectCount;

	static VkDescriptorSetLayout meshDSLayout;

	static GraphicsPipeline *pPipeline;

	std::string directory;

	std::map<uint32_t, Material*> materials;

	std::map<std::string, TextureImage*> textures;

	void processNode(aiNode *pAiNode, const aiScene *pAiScene);

	Mesh<Vertex>* processMesh(aiMesh *pAiMesh, const aiScene *pAiScene);

	Material* getMeshMaterial(uint32_t index, aiMaterial **ppAiMaterial);

	glm::vec4 getMaterialColor(aiMaterial *pAiMaterial, const char *key);

	void getMaterialTexture(aiTextureType type, aiMaterial *pAiMaterial, Material *pMaterial);

	TextureImage* loadMaterialTexture(aiMaterial *pAiMaterial, aiTextureType type);

	TextureImage* loadDefaultTexture(std::string path);
};

