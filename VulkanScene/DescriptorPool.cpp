#include <cassert>

#include "DescriptorPool.h"

// public:

DescriptorPool::DescriptorPool(Device *pDevice, uint32_t bufferCount, uint32_t textureCount, uint32_t setCount)
{
	this->pDevice = pDevice;

	VkDescriptorPoolSize uniformBuffersSize{
		VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,	// type;
		bufferCount,						// descriptorCount;
	};
	VkDescriptorPoolSize texturesSize{
		VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,	// type;
		textureCount,								// descriptorCount;
	};

	std::vector<VkDescriptorPoolSize> poolSizes{ uniformBuffersSize, texturesSize };

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

VkDescriptorSetLayout DescriptorPool::createDescriptorSetLayout(
	std::vector<VkShaderStageFlags> buffersShaderStages,
	std::vector<VkShaderStageFlags> texturesShaderStages
) const
{
	std::vector<VkDescriptorSetLayoutBinding> bindings;

	// add layout bindings of uniform buffers
	for (size_t i = 0; i < buffersShaderStages.size(); i++)
	{
		VkDescriptorSetLayoutBinding uniformBufferLayoutBinding{
			i,                                  // binding;
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,  // descriptorType;
			1,                                  // descriptorCount;
			buffersShaderStages[i],             // stageFlags;
			nullptr                             // pImmutableSamplers;
		};

		bindings.push_back(uniformBufferLayoutBinding);
	}

	// add layout bindings of textures
	for (size_t i = 0; i < texturesShaderStages.size(); i++)
	{
		VkDescriptorSetLayoutBinding textureLayoutBinding{
			buffersShaderStages.size() + i,             // binding;
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,  // descriptorType;
			1,                                          // descriptorCount;
			texturesShaderStages[i],                    // stageFlags;
			nullptr                                     // pImmutableSamplers;
		};

		bindings.push_back(textureLayoutBinding);
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

VkDescriptorSet DescriptorPool::getDescriptorSet(
    VkDescriptorSetLayout layout
) const
{
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

	return set;
}

void DescriptorPool::updateDescriptorSet(
	VkDescriptorSet set, 
	std::vector<Buffer*> buffers,
	std::vector<TextureImage*> textures
) const
{
	// write buffers in descriptor set
	std::vector<VkWriteDescriptorSet> buffersWrites;
	std::vector<VkDescriptorBufferInfo> buffersInfo(buffers.size());
	for (size_t i = 0; i < buffers.size(); i++)
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
	for (size_t i = 0; i < textures.size(); i++)
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

	// update descriptor set
	std::vector<VkWriteDescriptorSet> descriptorWrites(buffersWrites.begin(), buffersWrites.end());
	descriptorWrites.insert(descriptorWrites.end(), texturesWrites.begin(), texturesWrites.end());

	vkUpdateDescriptorSets(pDevice->device, descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
}
