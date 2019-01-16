#include "Position.h"

// public:

Position::Position(glm::vec3 pos) : pos(pos)
{
}

VkVertexInputBindingDescription Position::getBindingDescription(uint32_t binding)
{
	return VkVertexInputBindingDescription{
		binding,				
		sizeof(Position),			
		VK_VERTEX_INPUT_RATE_VERTEX	
	};
}

std::vector<VkVertexInputAttributeDescription> Position::getAttributeDescriptions(uint32_t binding, uint32_t locationOffset)
{
    const VkVertexInputAttributeDescription posDescription{
		locationOffset + 0,
		binding,	
		VK_FORMAT_R32G32B32_SFLOAT,
		offsetof(Position, pos)
	};
	return std::vector<VkVertexInputAttributeDescription>{ posDescription };
}
