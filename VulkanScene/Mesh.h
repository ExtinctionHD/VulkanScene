#pragma once

#include "Vertex.h"
#include <vector>
#include "Material.h"
#include "Buffer.h"
#include <vulkan/vulkan.h>
#include "MeshBase.h"

template <class T>
class Mesh : public MeshBase
{
public:
    Mesh(Device *device, const std::vector<T> &vertices, const std::vector<uint32_t> &indices, Material *material);

	~Mesh() = default;

	void clearHostVertices() override;

private:
	std::vector<T> vertices;
};

template <class T>
Mesh<T>::Mesh(Device *device, const std::vector<T> &vertices, const std::vector<uint32_t> &indices, Material *material)
    : MeshBase(device, indices, material)
{
    this->vertices = vertices;

    const VkDeviceSize size = vertices.size() * sizeof T;
	vertexBuffer = new Buffer(device, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, size);
	vertexBuffer->updateData(vertices.data(), vertices.size() * sizeof(vertices[0]), 0);
}

template <class T>
void Mesh<T>::clearHostVertices()
{
	vertices.clear();
}
