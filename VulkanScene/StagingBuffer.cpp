#include <cassert>

#include "StagingBuffer.h"

// public:

StagingBuffer::StagingBuffer(Device * pDevice, VkDeviceSize size)
{
	this->pDevice = pDevice;
	this->size = size;

	createBuffer(
		pDevice,
		size,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&stagingBuffer,
		&stagingMemory
	);
}

StagingBuffer::~StagingBuffer()
{
	vkFreeMemory(pDevice->getVk(), stagingMemory, nullptr);
	vkDestroyBuffer(pDevice->getVk(), stagingBuffer, nullptr);
}

void StagingBuffer::updateData(void * data, VkDeviceSize dataSize, VkDeviceSize offset)
{
	assert(offset + dataSize <= size);

	// update staging buffer
	void *bufferData;
	vkMapMemory(pDevice->getVk(), stagingMemory, offset, dataSize, 0, &bufferData);
	memcpy(bufferData, data, dataSize);
	vkUnmapMemory(pDevice->getVk(), stagingMemory);
}

void StagingBuffer::copyToImage(VkImage image, std::vector<VkBufferImageCopy> regions) const
{
	VkCommandBuffer commandBuffer = pDevice->beginOneTimeCommands();

	vkCmdCopyBufferToImage(commandBuffer, stagingBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, regions.size(), regions.data());

	pDevice->endOneTimeCommands(commandBuffer);
}

VkBuffer StagingBuffer::getBuffer() const
{
	return stagingBuffer;
}

VkDeviceSize StagingBuffer::getSize() const
{
	return size;
}

// protected:

void StagingBuffer::createBuffer(Device * pDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer * pBuffer, VkDeviceMemory * pMemory)
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

	VkResult result = vkCreateBuffer(pDevice->getVk(), &createInfo, nullptr, pBuffer);
	assert(result == VK_SUCCESS);

	allocateMemory(pDevice, pBuffer, pMemory, properties);

	vkBindBufferMemory(pDevice->getVk(), *pBuffer, *pMemory, 0);
}

void StagingBuffer::allocateMemory(Device * pDevice, VkBuffer * pBuffer, VkDeviceMemory * pMemory, VkMemoryPropertyFlags properties)
{
	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(pDevice->getVk(), *pBuffer, &memRequirements);

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

	VkResult result = vkAllocateMemory(pDevice->getVk(), &allocInfo, nullptr, pMemory);
	assert(result == VK_SUCCESS);
}
