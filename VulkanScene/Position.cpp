#include "Position.h"

// public:

Position::Position(glm::vec3 pos)
{
	this->pos = pos;
}

VkVertexInputBindingDescription Position::getBindingDescription(uint32_t binding)
{
	return VkVertexInputBindingDescription{
		binding,					// binding;
		sizeof(Position),			// stride;
		VK_VERTEX_INPUT_RATE_VERTEX	// inputRate;
	};
}

std::vector<VkVertexInputAttributeDescription> Position::getAttributeDescriptions(uint32_t binding, uint32_t locationOffset)
{
	VkVertexInputAttributeDescription posDescription{
		locationOffset + 0,
		binding,	
		VK_FORMAT_R32G32B32_SFLOAT,
		offsetof(Position, pos)
	};
	return std::vector<VkVertexInputAttributeDescription>{ posDescription };
}
