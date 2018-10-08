#include "PrimaryModel.h"

// public:

PrimaryModel::PrimaryModel(Device * pDevice, std::vector<PrimaryVertex*> vertices, std::vector<uint32_t> indices)
{
	this->pDevice = pDevice;
	this->vertices = vertices;
	this->indices = indices;

	for (PrimaryVertex *vertex : vertices)
	{
		findMaxMin(*vertex);
	}

	initSize(minVertex, maxVertex);
	initBuffers();
}

PrimaryModel::PrimaryModel(Device * pDevice, BaseModels base)
{
	/*switch (base)
	{
	case Model::CUBE:
		*this = Model(pDevice, )
			break;
	default:
		break;
	}*/
}

PrimaryModel::~PrimaryModel()
{
	delete(pIndexBuffer);
	delete(pVertexBuffer);
}

size_t PrimaryModel::getIndexCount() const
{
	return indices.size();
}

PrimaryModel& PrimaryModel::operator=(const PrimaryModel & model)
{
	pDevice = model.pDevice;
	vertices = model.vertices;
	indices = model.indices;

	size = model.size;
	minVertex = model.minVertex;
	maxVertex = model.maxVertex;

	initBuffers();

	return *this;
}

void PrimaryModel::normilize()
{
	glm::vec3 delta = glm::vec3(
		size.x / 2 - maxVertex.x,
		size.y / 2 - maxVertex.y,
		size.z / 2 - maxVertex.z
	);

	for (PrimaryVertex *vertex : vertices)
	{
		vertex->pos += delta;
	}

	maxVertex += delta;
	minVertex += delta;

	updateVertexBuffer();
}

// protected:

void PrimaryModel::findMaxMin(PrimaryVertex vertex)
{
	// find min vertex components
	if (vertex.pos.x < minVertex.x)
	{
		minVertex.x = vertex.pos.x;
	}
	if (vertex.pos.y < minVertex.y)
	{
		minVertex.y = vertex.pos.y;
	}
	if (vertex.pos.z < minVertex.z)
	{
		minVertex.z = vertex.pos.z;
	}

	// find max vertex components
	if (vertex.pos.x > maxVertex.x)
	{
		maxVertex.x = vertex.pos.x;
	}
	if (vertex.pos.y > maxVertex.y)
	{
		maxVertex.y = vertex.pos.y;
	}
	if (vertex.pos.z > maxVertex.z)
	{
		maxVertex.z = vertex.pos.z;
	}
}

void PrimaryModel::initSize(glm::vec3 minVertex, glm::vec3 maxVertex)
{
	size.x = maxVertex.x - minVertex.x;
	size.y = maxVertex.y - minVertex.y;
	size.z = maxVertex.z - minVertex.z;
}

void PrimaryModel::initBuffers()
{
	VkDeviceSize size = vertices.size() * vertices[0]->getSize();
	pVertexBuffer = new Buffer(pDevice, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_SHADER_STAGE_ALL, size);
	updateVertexBuffer();

	size = indices.size() * sizeof(indices[0]);
	pIndexBuffer = new Buffer(pDevice, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_SHADER_STAGE_ALL, size);
	pIndexBuffer->updateData(indices.data());
}

void PrimaryModel::updateVertexBuffer()
{
	std::vector<PrimaryVertex> tempVertices;
	for (PrimaryVertex *vertex : vertices)
	{
		tempVertices.push_back(*vertex);
	}

	pVertexBuffer->updateData(tempVertices.data());
}
