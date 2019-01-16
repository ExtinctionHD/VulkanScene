#pragma once

#include <vulkan/vulkan.h>
#include "Buffer.h"
#include "Material.h"

class MeshBase
{
public:
	virtual ~MeshBase();

	Material* getMaterial() const;

	void render(VkCommandBuffer commandBuffer, uint32_t instanceCount) const;

	void clearHostIndices();

	virtual void clearHostVertices() = 0;

protected:
	MeshBase(Device *device, const std::vector<uint32_t> &indices, Material *material);

	Material *material;

	Buffer *vertexBuffer;

	Buffer *indexBuffer;

	std::vector<uint32_t> indices;

	uint32_t indexCount;

};

