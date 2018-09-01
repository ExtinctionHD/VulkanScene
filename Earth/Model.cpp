#define TINYOBJLOADER_IMPLEMENTATION
#include "File.h"
#include "Logger.h"

#include "Model.h"

// public:

Model::Model(Device *pDevice, std::string filename)
{
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector <tinyobj::material_t> materials;

	std::string err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, filename.c_str(), File::getFileDir(filename).c_str()))
	{
		LOGGER_FATAL(Logger::getModelLoadingErrMsg(err));
	}
	if (!err.empty())
	{
		Logger::printInfo(Logger::getModelLoadingErrMsg(err));
	}

	initVectors(attrib, shapes);
	initSize(minVertex, maxVertex);
	initBuffers(pDevice);
}

Model::~Model()
{
	delete(pIndexBuffer);
	delete(pVertexBuffer);
}

// private:

size_t Model::getIndexCount() const
{
	return indices.size();
}

void Model::normilize()
{
	glm::vec3 delta = glm::vec3(
		size.x / 2 - maxVertex.x,
		size.y / 2 - maxVertex.y,
		size.z / 2 - maxVertex.z
	);

	for (Vertex& vertex : vertices)
	{
		vertex.pos += delta;
	}

	maxVertex += delta;
	minVertex += delta;

	pVertexBuffer->updateData(vertices.data());
}

void Model::initVectors(tinyobj::attrib_t attrib, std::vector<tinyobj::shape_t> shapes)
{
	minVertex = glm::vec3(max, max, max);
	maxVertex = glm::vec3(min, min, min);

	for (const auto& shape : shapes)
	{
		for (const auto& index : shape.mesh.indices)
		{
			Vertex vertex{ 
				{
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2]
				},
				{
					attrib.texcoords[2 * index.texcoord_index + 0],
					1 - attrib.texcoords[2 * index.texcoord_index + 1]
				}
			};

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

			vertices.push_back(vertex);
			indices.push_back(indices.size());
		}
	}
}

void Model::initSize(glm::vec3 minVertex, glm::vec3 maxVertex)
{
	size.x = maxVertex.x - minVertex.x;
	size.y = maxVertex.y - minVertex.y;
	size.z = maxVertex.z - minVertex.z;
}

void Model::initBuffers(Device *pDevice)
{
	VkDeviceSize size = vertices.size() * sizeof(vertices[0]);
	pVertexBuffer = new Buffer(pDevice, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_SHADER_STAGE_ALL, size);
	pVertexBuffer->updateData(vertices.data());

	size = indices.size() * sizeof(indices[0]);
	pIndexBuffer = new Buffer(pDevice, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_SHADER_STAGE_ALL, size);
	pIndexBuffer->updateData(indices.data());
}
