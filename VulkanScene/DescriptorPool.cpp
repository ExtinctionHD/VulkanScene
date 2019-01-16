#include <cassert>

#include "DescriptorPool.h"

// public:

DescriptorPool::DescriptorPool(Device *device, uint32_t bufferCount, uint32_t textureCount, uint32_t setCount)
{
	this->device = device;

    const VkDescriptorPoolSize uniformBuffersSize{
		VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		bufferCount,
	};
    const VkDescriptorPoolSize texturesSize{
		VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		textureCount,
	};

	std::vector<VkDescriptorPoolSize> poolSizes{ uniformBuffersSize, texturesSize };

	VkDescriptorPoolCreateInfo createInfo{
		VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		nullptr,										
		0,												
		setCount,										
		uint32_t(poolSizes.size()),								
		poolSizes.data(),								
	};

    const VkResult result = vkCreateDescriptorPool(device->get(), &createInfo, nullptr, &pool);
	assert(result == VK_SUCCESS);
}

DescriptorPool::~DescriptorPool()
{
	vkDestroyDescriptorPool(device->get(), pool, nullptr);
}

VkDescriptorSetLayout DescriptorPool::createDescriptorSetLayout(
	std::vector<VkShaderStageFlags> buffersShaderStages,
	std::vector<VkShaderStageFlags> texturesShaderStages) const
{
	std::vector<VkDescriptorSetLayoutBinding> bindings;

	for (size_t i = 0; i < buffersShaderStages.size(); i++)
	{
		VkDescriptorSetLayoutBinding uniformBufferLayoutBinding{
			uint32_t(i),                        
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			1,                                
			buffersShaderStages[i],           
			nullptr                           
		};

		bindings.push_back(uniformBufferLayoutBinding);
	}

	for (size_t i = 0; i < texturesShaderStages.size(); i++)
	{
		VkDescriptorSetLayoutBinding textureLayoutBinding{
			uint32_t(buffersShaderStages.size() + i),    
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 
			1,                                         
			texturesShaderStages[i],                   
			nullptr                                    
		};

		bindings.push_back(textureLayoutBinding);
	}

	VkDescriptorSetLayoutCreateInfo createInfo{
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		nullptr,											
		0,													
		uint32_t(bindings.size()),									
		bindings.data(),									
	};

	VkDescriptorSetLayout layout;
    const VkResult result = vkCreateDescriptorSetLayout(device->get(), &createInfo, nullptr, &layout);
	assert(result == VK_SUCCESS);

	return layout;
}

VkDescriptorSet DescriptorPool::getDescriptorSet(VkDescriptorSetLayout layout) const
{
	VkDescriptorSetAllocateInfo allocateInfo{
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		nullptr,										
		pool,											
		1,												
		&layout											
	};

	VkDescriptorSet set;
    const VkResult result = vkAllocateDescriptorSets(device->get(), &allocateInfo, &set);
	assert(result == VK_SUCCESS);

	return set;
}

void DescriptorPool::updateDescriptorSet(
	VkDescriptorSet set, 
	std::vector<Buffer*> buffers,
	std::vector<TextureImage*> textures) const
{
	std::vector<VkWriteDescriptorSet> buffersWrites;
	std::vector<VkDescriptorBufferInfo> buffersInfo(buffers.size());

	for (size_t i = 0; i < buffers.size(); i++)
	{
		buffersInfo[i] = {
			buffers[i]->get(),
			0,
			buffers[i]->getSize()
		};

		VkWriteDescriptorSet bufferWrite{
			VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			nullptr,							
			set,								
			uint32_t(i),									
			0,				
			1,		
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,	
			nullptr,							
			&buffersInfo[i],					
			nullptr,							
		};

		buffersWrites.push_back(bufferWrite);
	}

	std::vector<VkWriteDescriptorSet> texturesWrites;
	std::vector<VkDescriptorImageInfo> imagesInfo(textures.size());

	for (size_t i = 0; i < textures.size(); i++)
	{
		imagesInfo[i] = {
			textures[i]->getSampler(),				
			textures[i]->getView(),					
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		};

		VkWriteDescriptorSet textureWrite{
			VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			nullptr,									
			set,										
			uint32_t(buffers.size() + i),							
			0,											
			1,											
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,	
			&imagesInfo[i],								
			nullptr,									
			nullptr,									
		};

		texturesWrites.push_back(textureWrite);
	}

	std::vector<VkWriteDescriptorSet> descriptorWrites(buffersWrites.begin(), buffersWrites.end());
	descriptorWrites.insert(descriptorWrites.end(), texturesWrites.begin(), texturesWrites.end());

	vkUpdateDescriptorSets(device->get(), uint32_t(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}
