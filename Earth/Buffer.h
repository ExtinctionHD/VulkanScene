#pragma once

#include <vulkan/vulkan.h>
#include "Device.h"

// provides VkBuffer object and the ability to update it
class Buffer
{
public:
	Buffer(Device *pDevice, VkBufferUsageFlags usage, VkDeviceSize size);
	~Buffer();

	// main buffer object
	VkBuffer buffer;

	// size of this buffer
	VkDeviceSize size;

	// load data in this buffer
	// data size must be equals buffer size
	void updateData(void *data);

private:
	// device that provides this buffer and memory
	Device *pDevice;

	// main buffer memory
	VkDeviceMemory memory;

	// staging buffer allows update memory
	VkBuffer stagingBuffer;

	VkDeviceMemory stagingBufferMemory;

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

