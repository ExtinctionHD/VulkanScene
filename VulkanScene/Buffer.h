#pragma once

#include <vulkan/vulkan.h>
#include "Device.h"
#include "StagingBuffer.h"

// buffer with device local memory 
// updates using staging buffer
class Buffer : public StagingBuffer
{
public:
	Buffer(Device *device, VkBufferUsageFlags usage, VkDeviceSize size);
	~Buffer();

	VkBuffer get() const;

	void updateData(void *data, VkDeviceSize dataSize, VkDeviceSize offset) override;

private:
	VkBuffer buffer;

	VkDeviceMemory memory;
};

