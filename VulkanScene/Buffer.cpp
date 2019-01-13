#include "Buffer.h"

// public:

Buffer::Buffer(Device *pDevice, VkBufferUsageFlags usage, VkDeviceSize size) :
	StagingBuffer(pDevice, size)
{
	createBuffer(
		pDevice,
		size,
		usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		&buffer,
		&memory
	);
}

Buffer::~Buffer()
{
	vkFreeMemory(pDevice->getVk(), memory, nullptr);
	vkDestroyBuffer(pDevice->getVk(), buffer, nullptr);
}

VkBuffer Buffer::getBuffer() const
{
	return buffer;
}

void Buffer::updateData(void * data, VkDeviceSize dataSize, VkDeviceSize offset)
{
	// update staging buffer
	StagingBuffer::updateData(data, dataSize, offset);

	// update main buffer
	VkCommandBuffer commandBuffer = pDevice->beginOneTimeCommands();
	VkBufferCopy region{
		offset,		// srcOffset;
		offset,		// dstOffset;
		dataSize,	// size;
	};
	vkCmdCopyBuffer(commandBuffer, stagingBuffer, buffer, 1, &region);
	pDevice->endOneTimeCommands(commandBuffer);
}
