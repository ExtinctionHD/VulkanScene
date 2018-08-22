#include <vector>
#include "Logger.h"

#include "DescriptorSet.h"

// public:

DescriptorSet::DescriptorSet(VkDevice device)
{
	this->device = device;

	createLayout();
}

DescriptorSet::~DescriptorSet()
{
	vkDestroyDescriptorSetLayout(device, layout, nullptr);
}

// private:

void DescriptorSet::createLayout()
{
	VkDescriptorSetLayoutBinding uboLayoutBinding{
	   0,									// binding;
	   VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,	// descriptorType;
	   1,									// descriptorCount;
	   VK_SHADER_STAGE_VERTEX_BIT,			// stageFlags;
	   nullptr								// pImmutableSamplers;
	};

	VkDescriptorSetLayoutBinding samplerLayoutBinding{
		1,											// binding;
		VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,	// descriptorType;
		1,											// descriptorCount;
		VK_SHADER_STAGE_FRAGMENT_BIT,				// stageFlags;
		nullptr										// pImmutableSamplers;
	};

	std::vector<VkDescriptorSetLayoutBinding> bindings{
		uboLayoutBinding,
		samplerLayoutBinding
	};

	VkDescriptorSetLayoutCreateInfo createInfo{
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,	// sType;
		nullptr,												// pNext;
		0,														// flags;
		bindings.size(),										// bindingCount;
		bindings.data(),										// pBindings;
	};

	VkResult result = vkCreateDescriptorSetLayout(device, &createInfo, nullptr, &layout);
	if (result != VK_SUCCESS)
	{
		LOGGER_FATAL(Logger::FAILED_TO_CREATE_DS_LAYOUT);
	}
}
