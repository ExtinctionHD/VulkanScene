#include "MeshBase.h"

MeshBase::~MeshBase()
{
	delete vertexBuffer;
	delete indexBuffer;
}

Material * MeshBase::getMaterial() const
{
	return material;
}

void MeshBase::render(VkCommandBuffer commandBuffer, uint32_t instanceCount) const
{
	VkDeviceSize offset = 0;

	const VkBuffer vertexBuffer = this->vertexBuffer->get();
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer, &offset);

    const VkBuffer indexBuffer = this->indexBuffer->get();
	vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

	vkCmdDrawIndexed(commandBuffer, indexCount, instanceCount, 0, 0, 0);
}

void MeshBase::clearHostIndices()
{
	indices.clear();
}

MeshBase::MeshBase(Device *device, const std::vector<uint32_t> &indices, Material *material)
{
	this->indices = indices;
	this->material = material;

	indexCount = uint32_t(indices.size());

    const VkDeviceSize size = indexCount * sizeof uint32_t;
	indexBuffer = new Buffer(device, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, size);
	indexBuffer->updateData(indices.data(), indices.size() * sizeof(indices[0]), 0);
}
