#include "AssimpMesh.h"

AssimpMesh::AssimpMesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices, AssimpMaterial *pMaterial)
{
	this->vertices = vertices;
	this->indices = indices;
	this->pMaterial = pMaterial;
}

AssimpMesh::~AssimpMesh()
{
}
