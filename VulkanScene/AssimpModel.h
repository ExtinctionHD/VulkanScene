#pragma once

#include <assimp/scene.h>
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

	glm::vec3 getBaseSize() const;

	void scaleTo(glm::vec3 size);

	void scaleToX(float sizeX);

	void scaleToY(float sizeY);

	void scaleToZ(float sizeZ);

protected:
	VkVertexInputBindingDescription  getVertexInputBindingDescription(uint32_t inputBinding) override;

	std::vector<VkVertexInputAttributeDescription> getVertexInputAttributeDescriptions(uint32_t inputBinding) override;

private:
	std::string directory;

	std::map<std::string, TextureImage*> textures;

	glm::vec3 minPos = glm::vec3(std::numeric_limits<float>::infinity());

	glm::vec3 maxPos = glm::vec3(-std::numeric_limits<float>::infinity());

	void processNode(aiNode *pAiNode, const aiScene *pAiScene);

	Mesh<Vertex>* processMesh(aiMesh *pAiMesh, const aiScene *pAiScene);

	void initPosLimits(glm::vec3 pos);

	void initTangents(std::vector<Vertex>& vertices, std::vector<uint32_t> indices);

	Material* getMeshMaterial(uint32_t index, aiMaterial **ppAiMaterial);

	glm::vec4 getMaterialColor(aiMaterial *pAiMaterial, const char *key);

	TextureImage* loadMaterialTexture(aiMaterial *pAiMaterial, aiTextureType type);
};

