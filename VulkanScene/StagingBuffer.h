#pragma once

#include <vulkan/vulkan.h>
#include "Device.h"

// buffer that memory can be mapped into host memory
class StagingBuffer
{
public:
	StagingBuffer(Device *device, VkDeviceSize size);
	virtual ~StagingBuffer();

	virtual void updateData(const void *data, VkDeviceSize dataSize, VkDeviceSize offset);

	void copyToImage(VkImage image, std::vector<VkBufferImageCopy> regions) const;

	VkBuffer get() const;

	VkDeviceSize getSize() const;

protected:
	Device *device;

	VkBuffer stagingBuffer;

	VkDeviceSize size;

	VkDeviceMemory stagingMemory;

	static void createBuffer(
		Device *device,
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags properties,
		VkBuffer *buffer,
		VkDeviceMemory *memory);

	static void allocateMemory(
        Device *device,
        VkBuffer buffer,
        VkDeviceMemory *memory,
        VkMemoryPropertyFlags properties);
};

