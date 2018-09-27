#pragma once

#include "Buffer.h"
#include <vector>

class Mesh
{
public:
	~Mesh();

	Buffer *pVertexBuffer;
	Buffer *pIndexBuffer;

	// return number of indices
	size_t getIndexCount() const;

protected:
	Mesh() {}

	std::vector<uint32_t> indices;

};

