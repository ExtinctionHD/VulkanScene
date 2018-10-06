#pragma once

#include "Vertex.h"
#include <vector>
#include "Material.h"
#include "Buffer.h"
#include <vulkan/vulkan.h>
#include "MeshBase.h"

// mesh builded from vertices of T type
template <class T>
class Mesh : public MeshBase
{
public:
	Mesh(Device *pDevice, std::vector<T> vertices, std::vector<uint32_t> indices, Material *pMaterial)
	{
		this->vertices = vertices;
		this->indices = indices;
		this->pMaterial = pMaterial;

		initBuffers(pDevice);
	}

	~Mesh() {}

private:
	std::vector<T> vertices;

	void initBuffers(Device *pDevice)
	{
		VkDeviceSize size = vertices.size() * sizeof(vertices[0]);
		pVertexBuffer = new Buffer(pDevice, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_SHADER_STAGE_ALL, size);
		pVertexBuffer->updateData(vertices.data(), vertices.size() * sizeof(vertices[0]), 0);

		size = indices.size() * sizeof(indices[0]);
		pIndexBuffer = new Buffer(pDevice, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_SHADER_STAGE_ALL, size);
		pIndexBuffer->updateData(indices.data(), indices.size() * sizeof(indices[0]), 0);
	}
};

