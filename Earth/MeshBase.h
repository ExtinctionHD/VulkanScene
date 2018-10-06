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

	Buffer* getMaterialColorBuffer() const;

	std::vector<TextureImage*> getMaterialTextures() const;

protected:
	MeshBase() {}

	std::vector<uint32_t> indices;

	Material *pMaterial;

	Buffer *pVertexBuffer;

	Buffer *pIndexBuffer;

};

