#include "AssimpMesh.h"

// public:

template <class T>
AssimpMesh<T>::AssimpMesh(Device *pDevice, std::vector<T> vertices, std::vector<uint32_t> indices, AssimpMaterial *pMaterial)
{
	this->vertices = vertices;
	this->indices = indices;
	this->pMaterial = pMaterial;

	initBuffers(pDevice);
}

template <class T>
AssimpMesh<T>::~AssimpMesh()
{
	delete(pVertexBuffer);
	delete(pIndexBuffer);
}

template<class T>
VkBuffer AssimpMesh<T>::getVertexBuffer() const
{
	return pVertexBuffer->getBuffer();
}

template<class T>
VkBuffer AssimpMesh<T>::getIndexBuffer() const
{
	return pIndexBuffer->getBuffer();
}

template<class T>
uint32_t AssimpMesh<T>::getIndexCount() const
{
	return indices.size();
}

// private:

template<class T>
void AssimpMesh<T>::initBuffers(Device *pDevice)
{
	VkDeviceSize size = vertices.size() * sizeof(vertices[0]);
	pVertexBuffer = new Buffer(pDevice, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_SHADER_STAGE_ALL, size);
	pVertexBuffer->updateData(vertices.data(), vertices.size() * sizeof(vertices[0]), 0);

	size = indices.size() * sizeof(indices[0]);
	pIndexBuffer = new Buffer(pDevice, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_SHADER_STAGE_ALL, size);
	pIndexBuffer->updateData(indices.data(), indices.size() * sizeof(indices[0]), 0);
}
