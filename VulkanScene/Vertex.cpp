#include "Vertex.h"

// public:

VkVertexInputBindingDescription Vertex::getBindingDescription(uint32_t binding)
{
	return VkVertexInputBindingDescription{
		binding,	
		sizeof(Vertex),
		VK_VERTEX_INPUT_RATE_VERTEX
	};
}

std::vector<VkVertexInputAttributeDescription> Vertex::getAttributeDescriptions(uint32_t binding, uint32_t locationOffset)
{
    const VkVertexInputAttributeDescription posDescription{
		locationOffset + 0,	
		binding,	
		VK_FORMAT_R32G32B32_SFLOAT,
		offsetof(Vertex, pos)
	};

    const VkVertexInputAttributeDescription texDescription{
		locationOffset + 1,
		binding,	
		VK_FORMAT_R32G32_SFLOAT,
		offsetof(Vertex, uv)
	};

    const VkVertexInputAttributeDescription normalDescription{
		locationOffset + 2,	
		binding,	
		VK_FORMAT_R32G32B32_SFLOAT,
		offsetof(Vertex, normal)
	};

    const VkVertexInputAttributeDescription tangentDescription{
		locationOffset + 3,
		binding,	
		VK_FORMAT_R32G32B32_SFLOAT,
		offsetof(Vertex, tangent)
	};

	return std::vector<VkVertexInputAttributeDescription>{ posDescription, texDescription, normalDescription, tangentDescription };
}
