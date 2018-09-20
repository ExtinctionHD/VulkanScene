#include <vector>
#include "Logger.h"
#include "File.h"
#include <algorithm>
#include <array>

#include "DescriptorSet.h"

// public:

DescriptorSet::DescriptorSet(Device *pDevice)
{
	this->pDevice = pDevice;
}

DescriptorSet::~DescriptorSet()
{
	for (int i = 0; i < models.size(); i++)
	{
		delete(models[i]);
	}
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

void DescriptorSet::addBuffer(Buffer * pBuffer)
{
	uniformBuffers.push_back(pBuffer);
}

void DescriptorSet::addTexture(TextureImage * pTexture)
{
	textures.push_back(pTexture);
}

void DescriptorSet::removeBuffer(Buffer * pBuffer)
{
	std::remove(uniformBuffers.begin(), uniformBuffers.end(), pBuffer);
}

void DescriptorSet::removeTexture(TextureImage * pTexture)
{
	std::remove(textures.begin(), textures.end(), pTexture);
}

void DescriptorSet::addModel(ModelBase * pModel)
{
	models.push_back(pModel);
}

void DescriptorSet::removeModel(ModelBase * pModel)
{
	std::remove(models.begin(), models.end(), pModel);
}

void DescriptorSet::bind(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout) const
{
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &set, 0, nullptr);
}

void DescriptorSet::drawModels(VkCommandBuffer commandBuffer) const
{
	for (ModelBase *pModel : models)
	{
		VkBuffer vertexBuffers[] = { pModel->pVertexBuffer->getBuffer() };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(commandBuffer, pModel->pIndexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(commandBuffer, pModel->getIndexCount(), 1, 0, 0, 0);
	}
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
			i,															// binding;
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,							// descriptorType;
			1,															// descriptorCount;
			uniformBuffers[i]->shaderStage,	// stageFlags;
			nullptr														// pImmutableSamplers;
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
	std::vector<VkDescriptorBufferInfo> buffersInfo(uniformBuffers.size());
	for (int i = 0; i < uniformBuffers.size(); i++)
	{
		buffersInfo[i] = {
			uniformBuffers[i]->getBuffer(),	// buffer;
			0,								// offset;
			uniformBuffers[i]->getSize()	// range;
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

		uniformBuffersWrites.push_back(bufferWrite);
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
			getTextureBindingsOffset() + i,					// dstBinding;
			0,												// dstArrayElement;
			1,												// descriptorCount;
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,		// descriptorType;
			&imagesInfo[i],	// pImageInfo;
			nullptr,										// pBufferInfo;
			nullptr,										// pTexelBufferView;
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
