#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include <iostream>
#include <vector>

// vertex that contains only position
class PrimaryVertex
{
public:
	PrimaryVertex() {}
	PrimaryVertex(glm::vec3 pos);
	~PrimaryVertex();

	glm::vec3 pos;  // position of vertex (x, y, z)

	// description of whole vertex
	virtual VkVertexInputBindingDescription getBindingDescription(uint32_t binding);

	// description of each vertex component
	virtual  std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions(uint32_t binding);

	virtual uint32_t getSize();
};

