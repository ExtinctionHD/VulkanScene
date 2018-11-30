#pragma once

#include <vulkan/vulkan.h>
#include "Device.h"
#include "StagingBuffer.h"

// buffer with device local memory 
// updates using staging buffer
class Buffer : public StagingBuffer
{
public:
	Buffer(Device *pDevice, VkBufferUsageFlags usage, VkDeviceSize size);
	~Buffer();

	VkBuffer getBuffer() const;

	void updateData(void *pData, VkDeviceSize dataSize, VkDeviceSize offset) override;

private:
	// main buffer object
	VkBuffer buffer{};

	// main buffer memory
	VkDeviceMemory memory{};
};

