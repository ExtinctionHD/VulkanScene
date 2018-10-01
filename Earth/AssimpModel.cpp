#include "File.h"
#include "AssimpMaterial.h"
#include <algorithm>
#include <functional>

#include "AssimpModel.h"

AssimpModel::AssimpModel(Device *pDevice, const std::string& filename)
{
	this->pDevice = pDevice;

	directory = File::getFileDir(filename);

	Assimp::Importer importer;
	const aiScene *pScene = importer.ReadFile(
		filename, 
		aiProcess_Triangulate | 
		aiProcess_GenNormals |
		aiProcess_FlipUVs
	);
	if (!pScene)
	{
		throw std::runtime_error(importer.GetErrorString());
	}

	processNode(pScene->mRootNode, pScene);
}

AssimpModel::~AssimpModel()
{
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

void AssimpModel::processNode(aiNode *pNode, const aiScene *pScene)
{
	for (unsigned int i = 0; i < pNode->mNumMeshes; i++)
	{
		aiMesh *pMesh = pScene->mMeshes[pNode->mMeshes[i]];
		meshes.push_back(processMesh(pMesh, pScene));
	}

	for (unsigned int i = 0; i < pNode->mNumChildren; i++)
	{
		processNode(pNode->mChildren[i], pScene);
	}
}

AssimpMesh AssimpModel::processMesh(aiMesh * pMesh, const aiScene * pScene)
{
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	// add mesh vertices
	for (unsigned int i = 0; i < pMesh->mNumVertices; i++)
	{
		Vertex vertex;

		vertex.pos = glm::vec3(
			pMesh->mVertices[i].x,
			pMesh->mVertices[i].y,
			pMesh->mVertices[i].z
		);

		vertex.normal = glm::vec3(
			pMesh->mNormals[i].x,
			pMesh->mNormals[i].y,
			pMesh->mNormals[i].z
		);

		if (pMesh->mTextureCoords[0])
		{
			vertex.tex = glm::vec2(
				pMesh->mTextureCoords[0][i].x,
				pMesh->mTextureCoords[0][i].y
			);
		}
		else
		{
			vertex.tex = glm::vec2(0.0f, 0.0f);
		}

		vertices.push_back(vertex);
	}

	// add mesh indices
	for (unsigned int i = 0; i < pMesh->mNumFaces; i++)
	{
		aiFace face = pMesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++)
		{
			indices.push_back(face.mIndices[j]);
		}
	}

	AssimpMaterial *pMaterial = getMeshMaterial(pMesh->mMaterialIndex, pScene->mMaterials);

	return AssimpMesh(vertices, indices, pMaterial);
}

AssimpMaterial* AssimpModel::getMeshMaterial(uint32_t index, aiMaterial **aiMaterials)
{
	AssimpMaterial *pMaterial = new AssimpMaterial();

	if (materials.find(index) == materials.end())
	{
		aiMaterial *pAiMaterial = aiMaterials[index];

		pMaterial->colors.ambientColor = getMaterialColor(pAiMaterial, "COLOR_AMBIENT");
		pMaterial->colors.diffuseColor = getMaterialColor(pAiMaterial, "COLOR_DIFFUSE");
		pMaterial->colors.specularColor = getMaterialColor(pAiMaterial, "COLOR_SPECULAR");
		aiGetMaterialFloat(pAiMaterial, AI_MATKEY_OPACITY, &pMaterial->colors.opacity);

		getTexture(aiTextureType_DIFFUSE, pAiMaterial, pMaterial->pTexture);
		getTexture(aiTextureType_OPACITY, pAiMaterial, pMaterial->pOpacityMap);
		getTexture(aiTextureType_SPECULAR, pAiMaterial, pMaterial->pSpecularMap);
		getTexture(aiTextureType_NORMALS, pAiMaterial, pMaterial->pNormalMap);

		materials.insert(std::pair<uint32_t, AssimpMaterial*>(index, pMaterial));
	}
	else
	{
		pMaterial = materials[index];
	}

	return pMaterial;
}

glm::vec4 AssimpModel::getMaterialColor(aiMaterial *pMaterial, const char * key)
{
	aiColor4D color;
	aiGetMaterialColor(pMaterial, key, 0, 0, &color);
	return glm::vec4(color.r, color.g, color.b, color.a);
}

void AssimpModel::getTexture(aiTextureType type, aiMaterial *pMaterial, TextureImage *& pOutTexture)
{
	if (pMaterial->GetTextureCount(type))
	{
		pOutTexture = loadMaterialTexture(pMaterial, type);
	}
	else
	{
		pOutTexture = loadDefaultTexture(AssimpMaterial::getDefaultTexturePath(type));
	}
}

TextureImage* AssimpModel::loadMaterialTexture(aiMaterial *pMaterial, aiTextureType type)
{
	aiString path;
	pMaterial->GetTexture(type, 0, &path);

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
