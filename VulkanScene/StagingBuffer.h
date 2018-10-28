#pragma once

#include <vulkan/vulkan.h>
#include "Device.h"

// buffer that memory can be mapped into host memory
class StagingBuffer
{
public:
	StagingBuffer(Device *pDevice, VkDeviceSize size);
	virtual ~StagingBuffer();

	// load data in this buffer
	virtual void updateData(void *data, VkDeviceSize size, VkDeviceSize offset);

	void copyToImage(VkImage image, std::vector<VkBufferImageCopy> regions) const;

	VkBuffer getBuffer() const;

	VkDeviceSize getSize() const;

protected:
	// device that provides this buffer and memory
	Device * pDevice;

	// main buffer object
	VkBuffer stagingBuffer{};

	// size of this buffer
	VkDeviceSize size;

	// buffer memory
	VkDeviceMemory stagingMemory{};

	static void createBuffer(
		Device *pDevice,
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags properties,
		VkBuffer *pBuffer,
		VkDeviceMemory *pMemory
	);

	static void allocateMemory(Device *pDevice, VkBuffer *pBuffer, VkDeviceMemory *pMemory, VkMemoryPropertyFlags properties);
};

