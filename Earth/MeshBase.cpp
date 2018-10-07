#include "MeshBase.h"

MeshBase::~MeshBase()
{
	delete(pVertexBuffer);
	delete(pIndexBuffer);
}

VkBuffer MeshBase::getVertexBuffer() const
{
	return pVertexBuffer->getBuffer();
}

VkBuffer MeshBase::getIndexBuffer() const
{
	return pIndexBuffer->getBuffer();
}

uint32_t MeshBase::getIndexCount() const
{
	return indices.size();
}

Buffer* MeshBase::getMaterialColorBuffer() const
{
	return pMaterial->pColorsBuffer;
}

float MeshBase::getOpacity() const
{
	return pMaterial->colors.opacity;
}

std::vector<TextureImage*> MeshBase::getMaterialTextures() const
{
	return pMaterial->getTextures();
}
