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
	vkFreeMemory(pDevice->device, stagingMemory, nullptr);
	vkDestroyBuffer(pDevice->device, stagingBuffer, nullptr);
}

void StagingBuffer::updateData(void * data, VkDeviceSize dataSize, VkDeviceSize offset)
{
	if (offset + dataSize > size)
	{
		throw std::invalid_argument("Data size with offset can't be greate than buffer size");
	}

	// update staging buffer
	void *bufferData;
	vkMapMemory(pDevice->device, stagingMemory, offset, dataSize, 0, &bufferData);
	memcpy(bufferData, data, dataSize);
	vkUnmapMemory(pDevice->device, stagingMemory);
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

	VkResult result = vkCreateBuffer(pDevice->device, &createInfo, nullptr, pBuffer);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create buffer");
	}

	allocateMemory(pDevice, pBuffer, pMemory, properties);

	vkBindBufferMemory(pDevice->device, *pBuffer, *pMemory, 0);
}

void StagingBuffer::allocateMemory(Device * pDevice, VkBuffer * pBuffer, VkDeviceMemory * pMemory, VkMemoryPropertyFlags properties)
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
		throw std::runtime_error("Failed to allocate buffer memory");
	}
}
