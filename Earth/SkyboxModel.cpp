#include "SkyboxModel.h"



SkyboxModel::SkyboxModel(Device *pDevice, std::vector<Position> vertices, std::vector<uint32_t> indices)
{
	this->vertices = vertices;
	this->indices = indices;

	initBuffers(pDevice);
}

void SkyboxModel::initBuffers(Device *pDevice)
{
	VkDeviceSize size = vertices.size() * sizeof(vertices[0]);
	pVertexBuffer = new Buffer(pDevice, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_SHADER_STAGE_ALL, size);
	pVertexBuffer->updateData(vertices.data(), vertices.size() * sizeof(vertices[0]), 0);

	size = indices.size() * sizeof(indices[0]);
	pIndexBuffer = new Buffer(pDevice, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_SHADER_STAGE_ALL, size);
	pIndexBuffer->updateData(indices.data(), indices.size() * sizeof(indices[0]), 0);
}
