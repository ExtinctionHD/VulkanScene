#include "MeshBase.h"

MeshBase::~MeshBase()
{
	delete(pVertexBuffer);
	delete(pIndexBuffer);
}

void MeshBase::render(VkCommandBuffer commandBuffer, uint32_t instanceCount) const
{
	VkBuffer vertexBuffer = pVertexBuffer->get();
	VkDeviceSize offset = 0;
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer, &offset);

	VkBuffer indexBuffer = pIndexBuffer->get();
	vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

	vkCmdDrawIndexed(commandBuffer, indexCount, instanceCount, 0, 0, 0);
}

void MeshBase::clearHostIndices()
{
	indices.clear();
}
