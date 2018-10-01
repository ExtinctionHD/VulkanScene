#pragma once

#include "Vertex.h"
#include <vector>
#include "AssimpMaterial.h"

class AssimpMesh
{
public:
	AssimpMesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices, AssimpMaterial *pMaterial);
	~AssimpMesh();

private:
	std::vector<Vertex> vertices;

	std::vector<uint32_t> indices;

	AssimpMaterial *pMaterial;
};

