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

AssimpModel::AssimpModel(Device *device, const std::string &path, uint32_t count) :
	Model(device, count)
{
	directory = File::getDirectory(path);

	Assimp::Importer importer;
	const aiScene *aiScene = importer.ReadFile(
		File::getAbsolute(path),
		aiProcess_Triangulate | 
		aiProcess_GenNormals |
		aiProcess_FlipUVs);

	importer.ApplyPostProcessing(aiProcess_CalcTangentSpace);

	assert(aiScene);

	processNode(aiScene->mRootNode, aiScene);
}

AssimpModel::~AssimpModel()
{
	for (const auto &[name, texture] : textures)
	{
		delete texture;
	}
}

glm::vec3 AssimpModel::getBaseSize() const
{
	return maxPos - minPos;
}

// protected:

VkVertexInputBindingDescription AssimpModel::getVertexBindingDescription(uint32_t binding)
{
	return Vertex::getBindingDescription(binding);
}

std::vector<VkVertexInputAttributeDescription> AssimpModel::getVertexAttributeDescriptions(
    uint32_t binding,
    uint32_t locationOffset)
{
	return Vertex::getAttributeDescriptions(binding, locationOffset);
}

// private:

void AssimpModel::processNode(aiNode *aiNode, const aiScene *aiScene)
{
	for (unsigned int i = 0; i < aiNode->mNumMeshes; i++)
	{
		aiMesh *aiMesh = aiScene->mMeshes[aiNode->mMeshes[i]];

		MeshBase *mesh = processMesh(aiMesh, aiScene);

		if (mesh->getMaterial()->solid())
		{
			solidMeshes.push_back(mesh);
		}
		else
		{
			transparentMeshes.push_back(mesh);
		}
	}

	for (unsigned int i = 0; i < aiNode->mNumChildren; i++)
	{
		processNode(aiNode->mChildren[i], aiScene);
	}
}

Mesh<Vertex>* AssimpModel::processMesh(aiMesh *aiMesh, const aiScene *aiScene)
{
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	bool needInitTangents = false;

	for (unsigned int i = 0; i < aiMesh->mNumVertices; i++)
	{
		Vertex vertex;

		vertex.pos = glm::vec3(
			aiMesh->mVertices[i].x,
			aiMesh->mVertices[i].y,
			aiMesh->mVertices[i].z);

		initPosLimits(vertex.pos);

		vertex.normal = glm::vec3(
			aiMesh->mNormals[i].x,
			aiMesh->mNormals[i].y,
			aiMesh->mNormals[i].z);

		if (aiMesh->mTangents)
		{
			vertex.tangent = glm::vec3(
				aiMesh->mTangents[i].x,
				aiMesh->mTangents[i].y,
				aiMesh->mTangents[i].z);
		}
		else
		{
			needInitTangents = true;
		}

		if (aiMesh->mTextureCoords[0])
		{
			vertex.uv = glm::vec2(
				aiMesh->mTextureCoords[0][i].x,
				aiMesh->mTextureCoords[0][i].y);
		}
		else
		{
			vertex.uv = glm::vec2(0.0f, 0.0f);
		}

		vertices.push_back(vertex);
	}

	for (unsigned int i = 0; i < aiMesh->mNumFaces; i++)
	{
		aiFace face = aiMesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++)
		{
			indices.push_back(face.mIndices[j]);
		}
	}

	if (needInitTangents)
	{
		initTangents(vertices, indices);
	}

	Material *material = getMeshMaterial(aiMesh->mMaterialIndex, aiScene->mMaterials);

	return new Mesh<Vertex>(device, vertices, indices, material);
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

void AssimpModel::initTangents(std::vector<Vertex> &vertices, std::vector<uint32_t> indices) const
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

		glm::vec3 tan;

		if (deltaUV1.s != 0)
			tan = deltaPos / deltaUV1.s;
		else
			tan = deltaPos / 1.0f;

		tan = normalize(tan - dot(normal, tan) * normal);

		v0.tangent += tan;
		v1.tangent += tan;
		v2.tangent += tan;
	}

	for (auto &vertex : vertices)
    {
		vertex.tangent = normalize(vertex.tangent);
    }
}

Material* AssimpModel::getMeshMaterial(uint32_t index, aiMaterial *aiMaterials[])
{
    auto material = new Material(device);

	if (materials.find(index) == materials.end())
	{
        const auto aiMaterial = aiMaterials[index];

        // load material colors
		Material::Colors materialColors{};
		materialColors.diffuseColor = getMaterialColor(aiMaterial, "$clr.diffuse");
		materialColors.specularColor = getMaterialColor(aiMaterial, "$clr.specular");
		aiGetMaterialFloat(aiMaterial, AI_MATKEY_OPACITY, &materialColors.opacity);
		material->setColors(materialColors);

		for (auto type : Material::TEXTURES_ORDER)
		{
            if (type == aiTextureType_NORMALS)
            {
				if (aiMaterial->GetTextureCount(aiTextureType_HEIGHT))
				{
					material->addTexture(aiTextureType_NORMALS, loadMaterialTexture(aiMaterial, aiTextureType_HEIGHT));
				}
            }

			if (aiMaterial->GetTextureCount(type))
			{
				material->addTexture(type, loadMaterialTexture(aiMaterial, type));
			}
		}

		materials.insert({ index, material });
	}
	else
	{
		delete material;
		material = materials[index];
	}

	return material;
}

glm::vec4 AssimpModel::getMaterialColor(aiMaterial *aiMaterial, const char *key)
{
	aiColor4D color;
	if (aiGetMaterialColor(aiMaterial, key, 0, 0, &color) != aiReturn_SUCCESS)
	{
		return glm::vec4(1.0f);
	}
	return glm::vec4(color.r, color.g, color.b, color.a);
}

TextureImage* AssimpModel::loadMaterialTexture(aiMaterial *aiMaterial, aiTextureType type)
{
	aiString aiPath;
	assert(aiMaterial->GetTexture(type, 0, &aiPath) == aiReturn_SUCCESS);
	std::string path(aiPath.C_Str());

	TextureImage *texture;
	if (textures.find(path) == textures.end())
	{
        const VkFilter filter = type != aiTextureType_OPACITY ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;

		texture = new TextureImage(
			device, 
			{ File::getPath(directory, path) }, 
			1, 
			false, 
			filter,
			VK_SAMPLER_ADDRESS_MODE_REPEAT);

		textures.insert({ path, texture });
	}
	else
	{
		texture = textures[path];
	}

	return texture;
}
