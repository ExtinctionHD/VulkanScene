#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>
#include "AssimpMesh.h"
#include <vector>
#include <map>
#include "Device.h"

#pragma comment(lib, "assimp-vc140-mt.lib")

class AssimpModel
{
public:
	AssimpModel(Device *pDevice, const std::string& filename);
	~AssimpModel();

private:
	Device *pDevice;

	std::string directory;

	std::vector<AssimpMesh> meshes;

	std::map<uint32_t, AssimpMaterial*> materials;

	std::map<std::string, TextureImage*> textures;

	void processNode(aiNode *pNode, const aiScene *pScene);

	AssimpMesh processMesh(aiMesh *pMesh, const aiScene *pScene);

	AssimpMaterial* getMeshMaterial(uint32_t index, aiMaterial **aiMaterials);

	glm::vec4 getMaterialColor(aiMaterial *pMaterial, const char *key);

	void getTexture(aiTextureType type, aiMaterial *pMaterial, TextureImage*& pOutTexture);

	TextureImage* loadMaterialTexture(aiMaterial *pMaterial, aiTextureType type);

	TextureImage* loadDefaultTexture(std::string path);
};

