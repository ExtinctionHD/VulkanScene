#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include <vector>

class Position
{
public:
	Position() = default;

    Position(glm::vec3 pos);

    glm::vec3 pos{};

	// description of whole vertex
	static VkVertexInputBindingDescription getBindingDescription(uint32_t binding);

	// description of each vertex attribute
	static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions(uint32_t binding, uint32_t locationOffset);
};

