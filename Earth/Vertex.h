#pragma once

#include <vulkan/vulkan.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <vector>

class Vertex
{
public:
	Vertex(glm::vec3 pos, glm::vec2 tex);
	~Vertex();

	glm::vec3 pos;  // position of vertex (x, y, z)
	glm::vec2 tex;  // position of texture on this vertex (u, v)
	glm::vec3 normal = glm::vec3(0.0f, 0.0f, 0.0f);  // normal for vertex (x, y, z)

	// description of whole vertex
	static VkVertexInputBindingDescription getBindingDescription(uint32_t binding);

	static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions(uint32_t binding);
};

