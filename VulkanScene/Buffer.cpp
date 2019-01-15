#include "Buffer.h"

// public:

Buffer::Buffer(Device *device, VkBufferUsageFlags usage, VkDeviceSize size) : StagingBuffer(device, size)
{
	createBuffer(
		device,
		size,
		usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		&buffer,
		&memory);
}

Buffer::~Buffer()
{
	vkFreeMemory(device->get(), memory, nullptr);
	vkDestroyBuffer(device->get(), buffer, nullptr);
}

VkBuffer Buffer::get() const
{
	return buffer;
}

void Buffer::updateData(void *data, VkDeviceSize dataSize, VkDeviceSize offset)
{
	StagingBuffer::updateData(data, dataSize, offset);

	VkCommandBuffer commandBuffer = device->beginOneTimeCommands();
	VkBufferCopy region{
		offset,
		offset,	
		dataSize,
	};
	vkCmdCopyBuffer(commandBuffer, stagingBuffer, buffer, 1, &region);
	device->endOneTimeCommands(commandBuffer);
}
