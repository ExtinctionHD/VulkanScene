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
    Mesh(Device *device, std::vector<T> vertices, std::vector<uint32_t> indices, Material *pMaterial);

	~Mesh() {}

	void clearHostVertices() override;

private:
	std::vector<T> vertices;

    void initBuffers(Device *device);
};

template <class T>
Mesh<T>::Mesh(Device *device, std::vector<T> vertices, std::vector<uint32_t> indices, Material *pMaterial)
{
    this->vertices = vertices;
    this->indices = indices;
    this->pMaterial = pMaterial;

	indexCount = indices.size();

    initBuffers(device);
}

template <class T>
void Mesh<T>::clearHostVertices()
{
	vertices.clear();
}

template <class T>
void Mesh<T>::initBuffers(Device *device)
{
    VkDeviceSize size = vertices.size() * sizeof(vertices[0]);
    pVertexBuffer = new Buffer(device, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, size);
    pVertexBuffer->updateData(vertices.data(), vertices.size() * sizeof(vertices[0]), 0);

    size = indices.size() * sizeof(indices[0]);
    pIndexBuffer = new Buffer(device, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, size);
    pIndexBuffer->updateData(indices.data(), indices.size() * sizeof(indices[0]), 0);
}

