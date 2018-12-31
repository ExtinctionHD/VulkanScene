#pragma once

#include <vulkan/vulkan.h>
#include "Buffer.h"
#include "Material.h"

// contains the same members of any mesh
class MeshBase
{
public:
	virtual ~MeshBase();

	void render(VkCommandBuffer commandBuffer, uint32_t instanceCount) const;

	Material *pMaterial{};

	void clearHostIndices();

	virtual void clearHostVertices() = 0;

protected:
	MeshBase() = default;

    std::vector<uint32_t> indices;

	Buffer *pVertexBuffer{};

	Buffer *pIndexBuffer{};

	uint32_t indexCount{};

};

