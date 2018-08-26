#include "Vertex.h"

Vertex::Vertex()
{
}

Vertex::~Vertex()
{
}

VkVertexInputBindingDescription Vertex::getBindingDescription(uint32_t binding)
{
	return VkVertexInputBindingDescription{
		binding,					// binding;
		sizeof(Vertex),				// stride;
		VK_VERTEX_INPUT_RATE_VERTEX	// inputRate;
	};
}

std::vector<VkVertexInputAttributeDescription> Vertex::getAttributeDescriptions(uint32_t binding)
{
	VkVertexInputAttributeDescription posDescription{
		0,							// location;
		binding,					// binding;
		VK_FORMAT_R32G32B32_SFLOAT,	// format;
		offsetof(Vertex, pos)		// offset;
	};

	VkVertexInputAttributeDescription texDescription{
		1,							// location;
		binding,					// binding;
		VK_FORMAT_R32G32_SFLOAT,	// format;
		offsetof(Vertex, tex)		// offset;
	};

	return std::vector<VkVertexInputAttributeDescription>{ posDescription, texDescription };
}
