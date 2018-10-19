#pragma once

#include <vulkan/vulkan.h>
#include "Buffer.h"
#include "Material.h"
#include "TextureImage.h"

// contains the same members of any mesh
class MeshBase
{
public:
	virtual ~MeshBase();

	VkBuffer getVertexBuffer() const;

	VkBuffer getIndexBuffer() const;

	uint32_t getIndexCount() const;

	Material *pMaterial;

protected:
	MeshBase() {}

	std::vector<uint32_t> indices;

	Buffer *pVertexBuffer;

	Buffer *pIndexBuffer;

};

