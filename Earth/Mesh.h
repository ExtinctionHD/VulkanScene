#pragma once

#include "Vertex.h"
#include <vector>
#include "Material.h"
#include "Buffer.h"
#include <vulkan/vulkan.h>

template <class T>
class Mesh
{
public:
	Mesh(Device *pDevice, std::vector<T> vertices, std::vector<uint32_t> indices, Material *pMaterial)
	{
		this->vertices = vertices;
		this->indices = indices;
		this->pMaterial = pMaterial;

		initBuffers(pDevice);
	}

	~Mesh()
	{
		delete(pVertexBuffer);
		delete(pIndexBuffer);
	}

	VkBuffer getVertexBuffer() const
	{
		return pVertexBuffer->getBuffer();
	}

	VkBuffer getIndexBuffer() const
	{
		return pIndexBuffer->getBuffer();
	}

	uint32_t getIndexCount() const
	{
		return indices.size();
	}

	Buffer* getMaterialColorBuffer() const
	{
		return pMaterial->pColorsBuffer;
	}

	std::vector<TextureImage*> getMaterialTextures() const
	{
		return pMaterial->getTextures();
	}

private:
	std::vector<T> vertices;

	std::vector<uint32_t> indices;

	Material *pMaterial;

	Buffer *pVertexBuffer;

	Buffer *pIndexBuffer;

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

