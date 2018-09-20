#include "ModelBase.h"

ModelBase::~ModelBase()
{
	delete(pIndexBuffer);
	delete(pVertexBuffer);
}

size_t ModelBase::getIndexCount() const
{
	return indices.size();
}
