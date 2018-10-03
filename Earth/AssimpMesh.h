#pragma once

#include "Vertex.h"
#include <vector>
#include "AssimpMaterial.h"
#include "Buffer.h"
#include <vulkan/vulkan.h>

template <class T>
class AssimpMesh
{
public:
	AssimpMesh(Device *pDevice, std::vector<T> vertices, std::vector<uint32_t> indices, AssimpMaterial *pMaterial);
	~AssimpMesh();

	VkBuffer getVertexBuffer() const;

	VkBuffer getIndexBuffer() const;

	uint32_t getIndexCount() const;

private:
	std::vector<T> vertices;

	std::vector<uint32_t> indices;

	AssimpMaterial *pMaterial;

	Buffer *pVertexBuffer;

	Buffer *pIndexBuffer;

	void initBuffers(Device *pDevice);
};

