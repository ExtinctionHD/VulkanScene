#include "Logger.h"

#include "Buffer.h"

// public:

Buffer::Buffer(Device *pDevice, VkBufferUsageFlags usage, VkDeviceSize size)
{
	this->size = size;
	this->pDevice = pDevice;

	createBuffer(
		pDevice,
		size,
		usage | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&stagingBuffer,
		&stagingBufferMemory
	);

	createBuffer(
		pDevice,
		size,
		usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&stagingBuffer,
		&stagingBufferMemory
	);
}

Buffer::~Buffer()
{
	vkFreeMemory(pDevice->device, stagingBufferMemory, nullptr);
	vkDestroyBuffer(pDevice->device, stagingBuffer, nullptr);

	vkFreeMemory(pDevice->device, memory, nullptr);
	vkDestroyBuffer(pDevice->device, buffer, nullptr);
}

void Buffer::updateData(void * data)
{
	// update staging buffer
	void *bufferData;
	vkMapMemory(pDevice->device, stagingBufferMemory, 0, size, 0, &bufferData);
	memcpy(bufferData, data, size);
	vkUnmapMemory(pDevice->device, stagingBufferMemory);

	// update main buffer
	VkCommandBuffer commandBuffer = pDevice->beginOneTimeCommands();
	VkBufferCopy region{
		0,		// srcOffset;
		0,		// dstOffset;
		size,	// size;
	};
	vkCmdCopyBuffer(commandBuffer, stagingBuffer, buffer, 1, &region);
	pDevice->endOneTimeCommands(commandBuffer);
}

// private:

void Buffer::createBuffer(Device *pDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer *pBuffer, VkDeviceMemory *pMemory)
{
	VkBufferCreateInfo createInfo{
		VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,	// sType
		nullptr,								// pNext
		0,										// flags
		size,									// size
		usage,									// usage
		VK_SHARING_MODE_EXCLUSIVE,				// sharingMode
		0,										// queueFamilyIndexCount;
		nullptr,								// pQueueFamilyIndices;
	};

	VkResult result = vkCreateBuffer(pDevice->device, &createInfo, nullptr, pBuffer);
	if (result != VK_SUCCESS)
	{
		LOGGER_FATAL(Logger::FAILED_TO_CREATE_BUFFER);
	}

	allocateMemory(pDevice, pBuffer, pMemory, properties);

	vkBindBufferMemory(pDevice->device, *pBuffer, *pMemory, 0);
}

void Buffer::allocateMemory(Device * pDevice, VkBuffer *pBuffer, VkDeviceMemory *pMemory, VkMemoryPropertyFlags properties)
{
	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(pDevice->device, *pBuffer, &memRequirements);

	uint32_t memoryTypeIndex = pDevice->findMemoryTypeIndex(
		memRequirements.memoryTypeBits,
		properties
	);

	VkMemoryAllocateInfo allocInfo =
	{
		VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,	// sType
		nullptr,								// pNext
		memRequirements.size,					// allocationSize
		memoryTypeIndex,						// memoryTypeIndex
	};

	VkResult result = vkAllocateMemory(pDevice->device, &allocInfo, nullptr, pMemory);
	if (result != VK_SUCCESS)
	{
		LOGGER_FATAL(Logger::FAILED_TO_ALLOC_BUFFER_MEMORY);
	}
}
