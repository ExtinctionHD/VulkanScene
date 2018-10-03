#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>
#include "AssimpMesh.h"
#include <vector>
#include <map>
#include "Model.h"

#pragma comment(lib, "assimp-vc140-mt.lib")

class AssimpModel : public Model
{
public:
	AssimpModel(Device *pDevice, const std::string& filename);
	~AssimpModel();

private:
	std::string directory;

	std::vector<AssimpMesh<Vertex>*> meshes;

	std::map<uint32_t, AssimpMaterial*> materials;

	std::map<std::string, TextureImage*> textures;

	// methods:

	void processNode(aiNode *pAiNode, const aiScene *pAiScene);

	AssimpMesh<Vertex>* processMesh(aiMesh *pAiMesh, const aiScene *pAiScene);

	AssimpMaterial* getMeshMaterial(uint32_t index, aiMaterial **ppAiMaterial);

	glm::vec4 getMaterialColor(aiMaterial *pAiMaterial, const char *key);

	void getMaterialTexture(aiTextureType type, aiMaterial *pAiMaterial, AssimpMaterial *pMaterial);

	TextureImage* loadMaterialTexture(aiMaterial *pAiMaterial, aiTextureType type);

	TextureImage* loadDefaultTexture(std::string path);
};

