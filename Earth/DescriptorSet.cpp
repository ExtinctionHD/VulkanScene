#include <vector>
#include "Logger.h"
#include "File.h"

#include "DescriptorSet.h"

// public:

DescriptorSet::DescriptorSet(Device *pDevice)
{
	this->pDevice = pDevice;

	initUniformBuffers();
	initTextures();

	update();
}

DescriptorSet::~DescriptorSet()
{
	for (int i = 0; i < uniformBuffers.size(); i++)
	{
		delete(uniformBuffers[i]);
	}
	for (int i = 0; i < textures.size(); i++)
	{
		delete(textures[i]);
	}

	vkDestroyDescriptorPool(pDevice->device, pool, nullptr);
	vkDestroyDescriptorSetLayout(pDevice->device, layout, nullptr);
}

void DescriptorSet::update()
{
	createLayout();
	createDescriptorPool();
	createDescriptorSet();
}

// private:

void DescriptorSet::createLayout()
{
	// destroy old layout
	if (layout != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorSetLayout(pDevice->device, layout, nullptr);
	}

	std::vector<VkDescriptorSetLayoutBinding> bindings;

	// add layout bindings of uniform buffers
	for (int i = 0; i < uniformBuffers.size(); i++)
	{
		VkDescriptorSetLayoutBinding uniformBufferLayoutBinding{
			i,									// binding;
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,	// descriptorType;
			1,									// descriptorCount;
			VK_SHADER_STAGE_VERTEX_BIT,			// stageFlags;
			nullptr								// pImmutableSamplers;
		};

		bindings.push_back(uniformBufferLayoutBinding);
	}

	// add layout bindings of textures
	for (int i = 0; i < textures.size(); i++)
	{
		VkDescriptorSetLayoutBinding textureLayoutBinding{
			getTextureBindingsOffset() + i,				// binding;
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,	// descriptorType;
			1,											// descriptorCount;
			VK_SHADER_STAGE_FRAGMENT_BIT,				// stageFlags;
			nullptr										// pImmutableSamplers;
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

	VkResult result = vkCreateDescriptorSetLayout(pDevice->device, &createInfo, nullptr, &layout);
	if (result != VK_SUCCESS)
	{
		LOGGER_FATAL(Logger::FAILED_TO_CREATE_DS_LAYOUT);
	}
}

void DescriptorSet::createDescriptorPool()
{
	if (pool != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorPool(pDevice->device, pool, nullptr);
	}

	VkDescriptorPoolSize uniformBuffersSize{
		VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,	// type;
		uniformBuffers.size(),				// descriptorCount;
	};

	VkDescriptorPoolSize texturesSize{
		VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,	// type;
		textures.size(),							// descriptorCount;
	};

	std::vector<VkDescriptorPoolSize> poolSizes{ uniformBuffersSize, texturesSize };

	VkDescriptorPoolCreateInfo createInfo{
		VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,	// sType;
		nullptr,										// pNext;
		0,												// flags;
		1,												// maxSets;
		poolSizes.size(),								// poolSizeCount;
		poolSizes.data(),								// pPoolSizes;
	};

	VkResult result = vkCreateDescriptorPool(pDevice->device, &createInfo, nullptr, &pool);
	if (result != VK_SUCCESS)
	{
		LOGGER_FATAL(Logger::FAILED_TO_CREATE_DESCRIPTOR_POOL);
	}
}

void DescriptorSet::createDescriptorSet()
{
	VkDescriptorSetAllocateInfo allocateInfo{
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,	// sType;
		nullptr,										// pNext;
		pool,											// descriptorPool;
		1,												// descriptorSetCount;
		&layout											// pSetLayouts;
	};

	VkResult result = vkAllocateDescriptorSets(pDevice->device, &allocateInfo, &set);
	if (result != VK_SUCCESS)
	{
		LOGGER_FATAL(Logger::FAILED_TO_ALLOC_DESCRIPTOR_SET);
	}

	// write uniform buffers in descriptor set
	std::vector<VkWriteDescriptorSet> uniformBuffersWrites;
	for (int i = 0; i < uniformBuffers.size(); i++)
	{
		VkDescriptorBufferInfo bufferInfo{
			uniformBuffers[i]->buffer,	// buffer;
			0,							// offset;
			uniformBuffers[i]->size		// range;
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
			&bufferInfo,							// pBufferInfo;
			nullptr,								// pTexelBufferView;
		};

		uniformBuffersWrites.push_back(bufferWrite);
	}

	// write textures in descriptor set
	std::vector<VkWriteDescriptorSet> texturesWrites;
	for (int i = 0; i < textures.size(); i++)
	{
		VkDescriptorImageInfo imageInfo{
			textures[i]->sampler,						// sampler;
			textures[i]->view,							// imageView;
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,	// imageLayout;
		};

		VkWriteDescriptorSet textureWrite{
			VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,		// sType;
			nullptr,									// pNext;
			set,										// dstSet;
			getTextureBindingsOffset() + i,				// dstBinding;
			0,											// dstArrayElement;
			1,											// descriptorCount;
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,	// descriptorType;
			&imageInfo,									// pImageInfo;
			nullptr,									// pBufferInfo;
			nullptr,									// pTexelBufferView;
		};

		texturesWrites.push_back(textureWrite);
	}

	// update descriptor set
	std::vector<VkWriteDescriptorSet> descriptorWrites(uniformBuffersWrites.begin(), uniformBuffersWrites.end());
	descriptorWrites.insert(descriptorWrites.end(), texturesWrites.begin(), texturesWrites.end());
	vkUpdateDescriptorSets(pDevice->device, descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
}

uint32_t DescriptorSet::getTextureBindingsOffset()
{
	return uniformBuffers.size();
}

void DescriptorSet::initUniformBuffers()
{

}

void DescriptorSet::initTextures()
{
	const std::string EARTH_TEXTURE_PATH = File::getExeDir() + "/textures/earth.jpg";
	textures.push_back(new TextureImage(pDevice, EARTH_TEXTURE_PATH));
}
