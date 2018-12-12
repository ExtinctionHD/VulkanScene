#include "File.h"
#include "Material.h"
#include <functional>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>

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

	assert(pScene);

	processNode(pScene->mRootNode, pScene);
}

AssimpModel::~AssimpModel()
{
	// cleanup textures
	for (auto it = textures.begin(); it != textures.end(); ++it)
	{
		delete((*it).second);
	}
}

glm::vec3 AssimpModel::getBaseSize() const
{
	return maxPos - minPos;
}

// protected:

VkVertexInputBindingDescription AssimpModel::getVertexInputBindingDescription(uint32_t inputBinding)
{
	return Vertex::getBindingDescription(inputBinding);
}

std::vector<VkVertexInputAttributeDescription> AssimpModel::getVertexInputAttributeDescriptions(uint32_t inputBinding)
{
	return Vertex::getAttributeDescriptions(inputBinding);
}

// private:

void AssimpModel::processNode(aiNode *pAiNode, const aiScene *pAiScene)
{
	for (unsigned int i = 0; i < pAiNode->mNumMeshes; i++)
	{
		aiMesh *pAiMesh = pAiScene->mMeshes[pAiNode->mMeshes[i]];

		MeshBase* pMesh = processMesh(pAiMesh, pAiScene);

		if (pMesh->pMaterial->isSolid())
		{
			solidMeshes.push_back(pMesh);
		}
		else
		{
			transparentMeshes.push_back(pMesh);
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

		initPosLimits(vertex.pos);

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
			vertex.uv = glm::vec2(
				pAiMesh->mTextureCoords[0][i].x,
				pAiMesh->mTextureCoords[0][i].y
			);
		}
		else
		{
			vertex.uv = glm::vec2(0.0f, 0.0f);
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

void AssimpModel::initPosLimits(glm::vec3 pos)
{
	if (pos.x > maxPos.x)
	{
		maxPos.x = pos.x;
	}
	if (pos.y > maxPos.y)
	{
		maxPos.y = pos.y;
	}
	if (pos.z > maxPos.z)
	{
		maxPos.z = pos.z;
	}

	if (pos.x < minPos.x)
	{
		minPos.x = pos.x;
	}
	if (pos.y < minPos.y)
	{
		minPos.y = pos.y;
	}
	if (pos.z < minPos.z)
	{
		minPos.z = pos.z;
	}
}

void AssimpModel::initTangents(std::vector<Vertex>& vertices, std::vector<uint32_t> indices)
{
	for (unsigned int i = 0; i < indices.size(); i = i + 3) 
	{

		Vertex& v0 = vertices[indices[i]];
		Vertex& v1 = vertices[indices[i + 1]];
		Vertex& v2 = vertices[indices[i + 2]];

		glm::vec3 normal = glm::cross(v1.pos - v0.pos, v2.pos - v0.pos);

		glm::vec3 deltaPos;
		if (v0.pos == v1.pos)
			deltaPos = v2.pos - v0.pos;
		else
			deltaPos = v1.pos - v0.pos;

		glm::vec2 deltaUV1 = v1.uv - v0.uv;
		glm::vec2 deltaUV2 = v2.uv - v0.uv;

		glm::vec3 tan;
		glm::vec3 bin;

		if (deltaUV1.s != 0)
			tan = deltaPos / deltaUV1.s;
		else
			tan = deltaPos / 1.0f;

		tan = glm::normalize(tan - glm::dot(normal, tan) * normal);

		bin = glm::normalize(glm::cross(tan, normal));

		v0.tangent += tan;
		v1.tangent += tan;
		v2.tangent += tan;
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

		pMaterial->colors.diffuseColor = getMaterialColor(pAiMaterial, "$clr.diffuse");
		pMaterial->colors.specularColor = getMaterialColor(pAiMaterial, "$clr.specular");
		aiGetMaterialFloat(pAiMaterial, AI_MATKEY_OPACITY, &pMaterial->colors.opacity);

		pMaterial->updateColorsBuffer();

		for (aiTextureType type : Material::TEXTURES_ORDER)
		{
            if (type == aiTextureType_NORMALS)
            {
				if (pAiMaterial->GetTextureCount(aiTextureType_HEIGHT))
				{
					pMaterial->addTexture(aiTextureType_NORMALS, loadMaterialTexture(pAiMaterial, aiTextureType_HEIGHT));
				}
            }

			if (pAiMaterial->GetTextureCount(type))
			{
				pMaterial->addTexture(type, loadMaterialTexture(pAiMaterial, type));
			}
		}

		materials.insert({ index, pMaterial });
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

TextureImage* AssimpModel::loadMaterialTexture(aiMaterial *pAiMaterial, aiTextureType type)
{
	aiString path;
	assert(pAiMaterial->GetTexture(type, 0, &path) == aiReturn_SUCCESS);

	TextureImage *pTexture;
	if (textures.find(path.C_Str()) == textures.end())
	{
		pTexture = new TextureImage(pDevice, { directory + path.C_Str() }, 1);
		textures.insert({ path.C_Str(), pTexture });
	}
	else
	{
		pTexture = textures[path.C_Str()];
	}

	return pTexture;
}
