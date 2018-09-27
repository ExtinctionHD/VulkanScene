#include "Mesh.h"

Mesh::~Mesh()
{
	delete(pIndexBuffer);
	delete(pVertexBuffer);
}

size_t Mesh::getIndexCount() const
{
	return indices.size();
}
