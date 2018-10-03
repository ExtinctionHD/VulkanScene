#include "File.h"
#include "AssimpMaterial.h"
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
	if (!pScene)
	{
		throw std::runtime_error(importer.GetErrorString());
	}

	processNode(pScene->mRootNode, pScene);
}

AssimpModel::~AssimpModel()
{
	// cleanup materials
	for (auto it = meshes.begin(); it != meshes.end(); ++it)
	{
		delete(*it);
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

// private:

void AssimpModel::processNode(aiNode *pAiNode, const aiScene *pAiScene)
{
	for (unsigned int i = 0; i < pAiNode->mNumMeshes; i++)
	{
		aiMesh *pMesh = pAiScene->mMeshes[pAiNode->mMeshes[i]];
		meshes.push_back(processMesh(pMesh, pAiScene));
	}

	for (unsigned int i = 0; i < pAiNode->mNumChildren; i++)
	{
		processNode(pAiNode->mChildren[i], pAiScene);
	}
}

AssimpMesh<Vertex>* AssimpModel::processMesh(aiMesh * pAiMesh, const aiScene * pAiScene)
{
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

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

	AssimpMaterial *pMaterial = getMeshMaterial(pAiMesh->mMaterialIndex, pAiScene->mMaterials);

	return new AssimpMesh<Vertex>(pDevice, vertices, indices, pMaterial);
}

AssimpMaterial* AssimpModel::getMeshMaterial(uint32_t index, aiMaterial **ppAiMaterial)
{
	AssimpMaterial *pMaterial = new AssimpMaterial(pDevice);

	if (materials.find(index) == materials.end())
	{
		aiMaterial *pAiMaterial = ppAiMaterial[index];

		pMaterial->colors.ambientColor = getMaterialColor(pAiMaterial, "COLOR_AMBIENT");
		pMaterial->colors.diffuseColor = getMaterialColor(pAiMaterial, "COLOR_DIFFUSE");
		pMaterial->colors.specularColor = getMaterialColor(pAiMaterial, "COLOR_SPECULAR");
		aiGetMaterialFloat(pAiMaterial, AI_MATKEY_OPACITY, &pMaterial->colors.opacity);
		pMaterial->updateColorsBuffer();

		for (aiTextureType type : AssimpMaterial::TEXTURES_ORDER)
		{
			getMaterialTexture(type, pAiMaterial, pMaterial);
		}

		materials.insert(std::pair<uint32_t, AssimpMaterial*>(index, pMaterial));
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
	aiGetMaterialColor(pAiMaterial, key, 0, 0, &color);
	return glm::vec4(color.r, color.g, color.b, color.a);
}

void AssimpModel::getMaterialTexture(aiTextureType type, aiMaterial *pAiMaterial, AssimpMaterial *pMaterial)
{
	if (pAiMaterial->GetTextureCount(type))
	{
		pMaterial->addTexture(type, loadMaterialTexture(pAiMaterial, type));
	}
	else
	{
		pMaterial->addTexture(type, loadDefaultTexture(AssimpMaterial::getDefaultTexturePath(type)));
	}
}

TextureImage* AssimpModel::loadMaterialTexture(aiMaterial *pAiMaterial, aiTextureType type)
{
	aiString path;
	pAiMaterial->GetTexture(type, 0, &path);

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
