#include "PrimaryVertex.h"



PrimaryVertex::PrimaryVertex(glm::vec3 pos)
{
	this->pos = pos;
}

PrimaryVertex::~PrimaryVertex()
{
}

VkVertexInputBindingDescription PrimaryVertex::getBindingDescription(uint32_t binding)
{
	return VkVertexInputBindingDescription{
		binding,					// binding;
		sizeof(PrimaryVertex),		// stride;
		VK_VERTEX_INPUT_RATE_VERTEX	// inputRate;
	};
}

std::vector<VkVertexInputAttributeDescription> PrimaryVertex::getAttributeDescriptions(uint32_t binding)
{
	VkVertexInputAttributeDescription posDescription{
		0,								// location;
		binding,						// binding;
		VK_FORMAT_R32G32B32_SFLOAT,		// format;
		offsetof(PrimaryVertex, pos)	// offset;
	};
	return std::vector<VkVertexInputAttributeDescription>{ posDescription };
}

uint32_t PrimaryVertex::getSize()
{
	return sizeof(PrimaryVertex);
}
