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

protected:
	virtual VkVertexInputBindingDescription  getVertexInputBindingDescription(uint32_t inputBinding) override;

	virtual std::vector<VkVertexInputAttributeDescription> getVertexInputAttributeDescriptions(uint32_t inputBinding) override;

private:
	std::string directory;

	std::map<std::string, TextureImage*> textures;

	void processNode(aiNode *pAiNode, const aiScene *pAiScene);

	Mesh<Vertex>* processMesh(aiMesh *pAiMesh, const aiScene *pAiScene);

	void initTangents(std::vector<Vertex>& vertices, std::vector<uint32_t> indices);

	Material* getMeshMaterial(uint32_t index, aiMaterial **ppAiMaterial);

	glm::vec4 getMaterialColor(aiMaterial *pAiMaterial, const char *key);

	TextureImage* loadMaterialTexture(aiMaterial *pAiMaterial, aiTextureType type);
};

