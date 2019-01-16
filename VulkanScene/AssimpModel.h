#pragma once

#include <assimp/scene.h>
#include "Mesh.h"
#include <vector>
#include <map>
#include "Model.h"

class AssimpModel : public Model
{
public:
	AssimpModel(Device *device, const std::string &path, uint32_t count);

	~AssimpModel();

	glm::vec3 getBaseSize() const;

protected:
	VkVertexInputBindingDescription  getVertexBindingDescription(uint32_t binding) override;

	std::vector<VkVertexInputAttributeDescription> getVertexAttributeDescriptions(
        uint32_t binding,
        uint32_t locationOffset) override;

private:
	std::string directory;

	std::map<std::string, TextureImage*> textures;

	glm::vec3 minPos = glm::vec3(std::numeric_limits<float>::infinity());

	glm::vec3 maxPos = glm::vec3(-std::numeric_limits<float>::infinity());

	void processNode(aiNode *aiNode, const aiScene *aiScene);

	Mesh<Vertex>* processMesh(aiMesh *aiMesh, const aiScene *aiScene);

	void initPosLimits(glm::vec3 pos);

	void initTangents(std::vector<Vertex> &vertices, std::vector<uint32_t> indices) const;

	Material* getMeshMaterial(uint32_t index, aiMaterial *aiMaterials[]);

    static glm::vec4 getMaterialColor(aiMaterial *aiMaterial, const char *key);

	TextureImage* loadMaterialTexture(aiMaterial *aiMaterial, aiTextureType type);
};

