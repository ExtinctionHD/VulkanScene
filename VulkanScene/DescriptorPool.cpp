#include <cassert>

#include "DescriptorPool.h"

// public:

DescriptorPool::DescriptorPool(Device *pDevice, uint32_t bufferCount, uint32_t textureCount, uint32_t inputAttachmentCount, uint32_t setCount)
{
	this->pDevice = pDevice;

	VkDescriptorPoolSize uniformBuffersSize{
		VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,	// type;
		bufferCount							// descriptorCount;
	};
	VkDescriptorPoolSize texturesSize{
		VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,	// type;
		textureCount								// descriptorCount;
	};
	VkDescriptorPoolSize inputAttachmentSize{
		VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,	// type;
		inputAttachmentCount					// descriptorCount;
	};

	std::vector<VkDescriptorPoolSize> poolSizes{ uniformBuffersSize, texturesSize, inputAttachmentSize };

	VkDescriptorPoolCreateInfo createInfo{
		VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,	// sType;
		nullptr,										// pNext;
		0,												// flags;
		setCount,										// maxSets;
		poolSizes.size(),								// poolSizeCount;
		poolSizes.data(),								// pDescriptorPoolSizes;
	};

	VkResult result = vkCreateDescriptorPool(pDevice->device, &createInfo, nullptr, &pool);
	assert(result == VK_SUCCESS);
}

DescriptorPool::~DescriptorPool()
{
	vkDestroyDescriptorPool(pDevice->device, pool, nullptr);
}

VkDescriptorSet DescriptorPool::getDescriptorSet(std::vector<Buffer*> buffers, std::vector<TextureImage*> textures, std::vector<Image*> inputAttachments, bool createLayout, VkDescriptorSetLayout& layout)
{
	if (createLayout)
	{
		layout = createDescriptorSetLayout(buffers, textures, inputAttachments);
	}

	VkDescriptorSetAllocateInfo allocateInfo{
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,	// sType;
		nullptr,										// pNext;
		pool,											// descriptorPool;
		1,												// descriptorSetCount;
		&layout											// pSetLayouts;
	};

	VkDescriptorSet set;
	VkResult result = vkAllocateDescriptorSets(pDevice->device, &allocateInfo, &set);
	assert(result == VK_SUCCESS);

	// write buffers in descriptor set
	std::vector<VkWriteDescriptorSet> buffersWrites;
	std::vector<VkDescriptorBufferInfo> buffersInfo(buffers.size());
	for (int i = 0; i < buffers.size(); i++)
	{
		buffersInfo[i] = {
			buffers[i]->getBuffer(),	// buffer;
			0,							// offset;
			buffers[i]->getSize()		// range;
		};

		VkWriteDescriptorSet bufferWrite{
			VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,	// sType;
			nullptr,								// pNext;
			set,									// dstSet;
			i,										// dstBinding;
			0,										// dstArrayElement;
			1,										// descriptorCount;
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,		// descriptorType;
			nullptr,								// pImageInfo;
			&buffersInfo[i],						// pBufferInfo;
			nullptr,								// pTexelBufferView;
		};

		buffersWrites.push_back(bufferWrite);
	}

	// write textures in descriptor set
	std::vector<VkWriteDescriptorSet> texturesWrites;
	std::vector<VkDescriptorImageInfo> imagesInfo(textures.size());
	for (int i = 0; i < textures.size(); i++)
	{
		imagesInfo[i] = {
			textures[i]->sampler,						// sampler;
			textures[i]->view,							// imageView;
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,	// imageLayout;
		};

		VkWriteDescriptorSet textureWrite{
			VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,			// sType;
			nullptr,										// pNext;
			set,											// dstSet;
			buffers.size() + i,								// dstBinding;
			0,												// dstArrayElement;
			1,												// descriptorCount;
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,		// descriptorType;
			&imagesInfo[i],									// pImageInfo;
			nullptr,										// pBufferInfo;
			nullptr,										// pTexelBufferView;
		};

		texturesWrites.push_back(textureWrite);
	}

	// write input attachments in descriptor set
	std::vector<VkWriteDescriptorSet> inputAttachmentsWrites;
	std::vector<VkDescriptorImageInfo> inputAttachmentsInfo(inputAttachments.size());
	for (int i = 0; i < inputAttachments.size(); i++)
	{
		inputAttachmentsInfo[i] = {
			VK_NULL_HANDLE,								// sampler;
			inputAttachments[i]->view,					// imageView;
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,	// imageLayout;
		};

		VkWriteDescriptorSet inputAttachmentWrite{
			VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,			// sType;
			nullptr,										// pNext;
			set,											// dstSet;
			buffers.size() + textures.size() + i,			// dstBinding;
			0,												// dstArrayElement;
			1,												// descriptorCount;
			VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,			// descriptorType;
			&inputAttachmentsInfo[i],						// pImageInfo;
			nullptr,										// pBufferInfo;
			nullptr,										// pTexelBufferView;
		};

		inputAttachmentsWrites.push_back(inputAttachmentWrite);
	}

	// update descriptor set
	vkUpdateDescriptorSets(pDevice->device, buffersWrites.size(), buffersWrites.data(), 0, nullptr);
	vkUpdateDescriptorSets(pDevice->device, texturesWrites.size(), texturesWrites.data(), 0, nullptr);
	vkUpdateDescriptorSets(pDevice->device, inputAttachmentsWrites.size(), inputAttachmentsWrites.data(), 0, nullptr);

	return set;
}

VkDescriptorSetLayout DescriptorPool::createDescriptorSetLayout(std::vector<Buffer*> buffers, std::vector<TextureImage*> textures, std::vector<Image*> inputAttachments)
{
	std::vector<VkDescriptorSetLayoutBinding> bindings;

	// add layout bindings of uniform buffers
	for (int i = 0; i < buffers.size(); i++)
	{
		VkDescriptorSetLayoutBinding uniformBufferLayoutBinding{
			i,									// binding;
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,	// descriptorType;
			1,									// descriptorCount;
			buffers[i]->shaderStage,			// stageFlags;
			nullptr								// pImmutableSamplers;
		};

		bindings.push_back(uniformBufferLayoutBinding);
	}

	// add layout bindings of textures
	for (int i = 0; i < textures.size(); i++)
	{
		VkDescriptorSetLayoutBinding textureLayoutBinding{
			buffers.size() + i,							// binding;
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,	// descriptorType;
			1,											// descriptorCount;
			VK_SHADER_STAGE_FRAGMENT_BIT,				// stageFlags;
			nullptr										// pImmutableSamplers;
		};

		bindings.push_back(textureLayoutBinding);
	}

	for (int i = 0; i < inputAttachments.size(); i++)
	{
		VkDescriptorSetLayoutBinding inputAttachmentLayoutBinding{
			buffers.size() + textures.size() + i,		// binding;
			VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,		// descriptorType;
			1,											// descriptorCount;
			VK_SHADER_STAGE_FRAGMENT_BIT,				// stageFlags;
			nullptr										// pImmutableSamplers;
		};

		bindings.push_back(inputAttachmentLayoutBinding);
	}

	VkDescriptorSetLayoutCreateInfo createInfo{
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,	// sType;
		nullptr,												// pNext;
		0,														// flags;
		bindings.size(),										// bindingCount;
		bindings.data(),										// pBindings;
	};

	VkDescriptorSetLayout layout;
	VkResult result = vkCreateDescriptorSetLayout(pDevice->device, &createInfo, nullptr, &layout);
	assert(result == VK_SUCCESS);

	return layout;
}
