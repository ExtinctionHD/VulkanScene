#include "File.h"
#include "Material.h"
#include <algorithm>
#include <functional>

#include "AssimpModel.h"

// public:

AssimpModel::AssimpModel(Device *pDevice, const std::string& filename) :
	Model(pDevice)
{
	directory = File::getFileDir(filename);

	Assimp::Importer importer;
	const aiScene *pScene = importer.ReadFile(
		filename, 
		aiProcess_Triangulate | 
		aiProcess_GenNormals |
		aiProcess_FlipUVs
	);
	importer.ApplyPostProcessing(aiProcess_CalcTangentSpace);
	if (!pScene)
	{
		throw std::runtime_error(importer.GetErrorString());
	}

	processNode(pScene->mRootNode, pScene);

	objectCount++;
}

AssimpModel::~AssimpModel()
{
	objectCount--;

	if (objectCount == 0)
	{
		if (meshDSLayout != VK_NULL_HANDLE)
		{
			vkDestroyDescriptorSetLayout(pDevice->device, meshDSLayout, nullptr);
			meshDSLayout = VK_NULL_HANDLE;
		}
	}

	// cleanup materials
	for (auto it = materials.begin(); it != materials.end(); ++it)
	{
		delete((*it).second);
	}

	// cleanup textures
	for (auto it = textures.begin(); it != textures.end(); ++it)
	{
		delete((*it).second);
	}
}

void AssimpModel::createPipeline(Device *pDevice, std::vector<VkDescriptorSetLayout> layouts, RenderPass * pRenderPass)
{
	layouts.push_back(mvpDSLayout);
	layouts.push_back(meshDSLayout);

	if (pPipeline == nullptr)
	{
		delete(pPipeline);
	}

	const uint32_t inputBinding = 0;

	pPipeline = new GraphicsPipeline(
		pDevice->device,
		layouts,
		pRenderPass,
		{
			new ShaderModule(pDevice->device, SHADER_FILES[ShaderTypes::vert], VK_SHADER_STAGE_VERTEX_BIT),
			new ShaderModule(pDevice->device, SHADER_FILES[ShaderTypes::frag], VK_SHADER_STAGE_FRAGMENT_BIT)
		},
		Vertex::getBindingDescription(inputBinding),
		Vertex::getAttributeDescriptions(inputBinding)
	);
}

void AssimpModel::recreatePipeline(RenderPass * pRenderPass)
{
	if (pPipeline == nullptr)
	{
		throw std::runtime_error("Pipeline not created yet");
	}

	pPipeline->recreate(pRenderPass);
}

void AssimpModel::destroyPipeline()
{
	if (pPipeline != nullptr)
	{
		delete(pPipeline);
		pPipeline = nullptr;
	}
}

// protected:

VkDescriptorSetLayout& AssimpModel::getMeshDSLayout()
{
	return meshDSLayout;
}

GraphicsPipeline * AssimpModel::getPipeline()
{
	return pPipeline;
}

// private:

const std::vector<std::string> AssimpModel::SHADER_FILES = {
	File::getExeDir() + "shaders/main/vert.spv",
	File::getExeDir() + "shaders/main/frag.spv"
};

uint32_t AssimpModel::objectCount = 0;

VkDescriptorSetLayout AssimpModel::meshDSLayout = VK_NULL_HANDLE;

GraphicsPipeline* AssimpModel::pPipeline = nullptr;

void AssimpModel::processNode(aiNode *pAiNode, const aiScene *pAiScene)
{
	for (unsigned int i = 0; i < pAiNode->mNumMeshes; i++)
	{
		aiMesh *pAiMesh = pAiScene->mMeshes[pAiNode->mMeshes[i]];

		MeshBase* pMesh = processMesh(pAiMesh, pAiScene);

		if (pMesh->getOpacity() == 1.0f)
		{
			meshes.insert(meshes.begin(), { pMesh });
		}
		else
		{
			meshes.push_back(pMesh);
		}
	}

	for (unsigned int i = 0; i < pAiNode->mNumChildren; i++)
	{
		processNode(pAiNode->mChildren[i], pAiScene);
	}
}

Mesh<Vertex>* AssimpModel::processMesh(aiMesh * pAiMesh, const aiScene * pAiScene)
{
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	bool needInitTangets = false;

	// add mesh vertices
	for (unsigned int i = 0; i < pAiMesh->mNumVertices; i++)
	{
		Vertex vertex;

		vertex.pos = glm::vec3(
			pAiMesh->mVertices[i].x,
			pAiMesh->mVertices[i].y,
			pAiMesh->mVertices[i].z
		);

		vertex.normal = glm::vec3(
			pAiMesh->mNormals[i].x,
			pAiMesh->mNormals[i].y,
			pAiMesh->mNormals[i].z
		);

		if (pAiMesh->mTangents != nullptr)
		{
			vertex.tangent = glm::vec3(
				pAiMesh->mTangents[i].x,
				pAiMesh->mTangents[i].y,
				pAiMesh->mTangents[i].z
			);
		}
		else
		{
			needInitTangets = true;
		}

		if (pAiMesh->mTextureCoords[0])
		{
			vertex.tex = glm::vec2(
				pAiMesh->mTextureCoords[0][i].x,
				pAiMesh->mTextureCoords[0][i].y
			);
		}
		else
		{
			vertex.tex = glm::vec2(0.0f, 0.0f);
		}

		vertices.push_back(vertex);
	}

	// add mesh indices
	for (unsigned int i = 0; i < pAiMesh->mNumFaces; i++)
	{
		aiFace face = pAiMesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++)
		{
			indices.push_back(face.mIndices[j]);
		}
	}

	if (needInitTangets)
	{
		initTangents(vertices, indices);
	}

	Material *pMaterial = getMeshMaterial(pAiMesh->mMaterialIndex, pAiScene->mMaterials);

	return new Mesh<Vertex>(pDevice, vertices, indices, pMaterial);
}

void AssimpModel::initTangents(std::vector<Vertex>& vertices, std::vector<uint32_t> indices)
{
	for (uint32_t i = 0; i < indices.size(); i += 3)
	{
		Vertex& v0 = vertices[indices[i]];
		Vertex& v1 = vertices[indices[i + 1]];
		Vertex& v2 = vertices[indices[i + 2]];

		glm::vec3 edge1 = v1.pos - v0.pos;
		glm::vec3 edge2 = v2.pos - v0.pos;

		float deltaU1 = v1.tex.x - v0.tex.x;
		float deltaV1 = v1.tex.y - v0.tex.y;
		float deltaU2 = v2.tex.x - v0.tex.x;
		float deltaV2 = v2.tex.y - v0.tex.y;

		float f = 1.0f / (deltaU1 * deltaV2 - deltaU2 * deltaV1);

		glm::vec3 tangent;

		tangent.x = f * (deltaV2 * edge1.x - deltaV1 * edge2.x);
		tangent.y = f * (deltaV2 * edge1.y - deltaV1 * edge2.y);
		tangent.z = f * (deltaV2 * edge1.z - deltaV1 * edge2.z);

		v0.tangent += tangent;
		v1.tangent += tangent;
		v2.tangent += tangent;
	}

	for (uint32_t i = 0; i < vertices.size(); i++)
	{
		vertices[i].tangent = glm::normalize(vertices[i].tangent);
	}
}

Material* AssimpModel::getMeshMaterial(uint32_t index, aiMaterial **ppAiMaterial)
{
	Material *pMaterial = new Material(pDevice);

	if (materials.find(index) == materials.end())
	{
		aiMaterial *pAiMaterial = ppAiMaterial[index];

		pMaterial->colors.ambientColor = getMaterialColor(pAiMaterial, "$clr.ambient");
		pMaterial->colors.diffuseColor = getMaterialColor(pAiMaterial, "$clr.diffuse");
		pMaterial->colors.specularColor = getMaterialColor(pAiMaterial, "$clr.specular");
		if (aiGetMaterialFloat(pAiMaterial, AI_MATKEY_OPACITY, &pMaterial->colors.opacity) != aiReturn_SUCCESS)
		{
			pMaterial->colors.opacity = 1.0f;
		}
		pMaterial->updateColorsBuffer();

		for (aiTextureType type : Material::TEXTURES_ORDER)
		{
			getMaterialTexture(type, pAiMaterial, pMaterial);
		}

		materials.insert(std::pair<uint32_t, Material*>(index, pMaterial));
	}
	else
	{
		pMaterial = materials[index];
	}

	return pMaterial;
}

glm::vec4 AssimpModel::getMaterialColor(aiMaterial *pAiMaterial, const char * key)
{
	aiColor4D color;
	if (aiGetMaterialColor(pAiMaterial, key, 0, 0, &color) != aiReturn_SUCCESS)
	{
		return glm::vec4(1.0f);
	}
	return glm::vec4(color.r, color.g, color.b, color.a);
}

void AssimpModel::getMaterialTexture(aiTextureType type, aiMaterial *pAiMaterial, Material *pMaterial)
{
	if (pAiMaterial->GetTextureCount(type))
	{
		pMaterial->addTexture(type, loadMaterialTexture(pAiMaterial, type));
	}
	else
	{
		pMaterial->addTexture(type, loadDefaultTexture(Material::getDefaultTexturePath(type)));
	}
}

TextureImage* AssimpModel::loadMaterialTexture(aiMaterial *pAiMaterial, aiTextureType type)
{
	aiString path;
	if (pAiMaterial->GetTexture(type, 0, &path) != aiReturn_SUCCESS)
	{
		throw std::runtime_error("Failed to get material texture");
	}

	TextureImage *pTexture;
	if (textures.find(path.C_Str()) == textures.end())
	{
		pTexture = new TextureImage(pDevice, { directory + path.C_Str() }, 1);
		textures.insert(std::pair<std::string, TextureImage*>(path.C_Str(), pTexture));
	}
	else
	{
		pTexture = textures[path.C_Str()];
	}

	return pTexture;
}

TextureImage * AssimpModel::loadDefaultTexture(std::string path)
{
	TextureImage *pTexture;
	if (textures.find(path) == textures.end())
	{
		pTexture = new TextureImage(pDevice, { path }, 1);
		textures.insert(std::pair<std::string, TextureImage*>(path, pTexture));
	}
	else
	{
		pTexture = textures[path];
	}

	return pTexture;
}
