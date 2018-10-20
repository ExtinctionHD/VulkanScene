#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include <vector>

struct Vertex
{
	glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f);		// position of vertex (x, y, z)
	glm::vec2 tex = glm::vec3(0.0f, 0.0f, 0.0f);		// position of texture on this vertex (u, v)
	glm::vec3 normal = glm::vec3(0.0f, 0.0f, 0.0f);		// average normal for triangles that include this vertex (x, y, z)
	glm::vec3 tangent = glm::vec3(0.0f, 0.0f, 0.0f);	// texture vector u in local space (x, y, z)

	// description of whole vertex
	static VkVertexInputBindingDescription getBindingDescription(uint32_t binding);

	// description of each vertex attribute
	static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions(uint32_t binding);
};

