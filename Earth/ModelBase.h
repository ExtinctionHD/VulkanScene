#pragma once

#include "Buffer.h"
#include <vector>

class ModelBase
{
public:
	ModelBase() {}
	~ModelBase();

	Buffer *pVertexBuffer;
	Buffer *pIndexBuffer;

	// return number of indices
	size_t getIndexCount() const;

protected:
	std::vector<uint32_t> indices;

};

