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

class AssimpModel : public Model
{
public:
	AssimpModel(Device *pDevice, const std::string& filename);
	~AssimpModel();

protected:
	void virtual initMeshDescriptorSets(DescriptorPool *pPool) override;

private:
	static uint32_t objectCount;

	static VkDescriptorSetLayout meshDSLayout;

	std::string directory;

	std::vector<Mesh<Vertex>*> meshes;

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

