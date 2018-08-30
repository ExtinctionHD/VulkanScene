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
	initBuffers(pDevice);
}

Model::~Model()
{
	delete(pIndexBuffer);
	delete(pVertexBuffer);
}

// private:

size_t Model::getIndexCount()
{
	return indices.size();
}

void Model::initVectors(tinyobj::attrib_t attrib, std::vector<tinyobj::shape_t> shapes)
{
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

			vertices.push_back(vertex);
			indices.push_back(indices.size());
		}
	}
}

void Model::initBuffers(Device *pDevice)
{
	VkDeviceSize size = vertices.size() * sizeof(vertices[0]);
	pVertexBuffer = new Buffer(pDevice, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, size);
	pVertexBuffer->updateData(vertices.data());

	size = indices.size() * sizeof(indices[0]);
	pIndexBuffer = new Buffer(pDevice, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, size);
	pIndexBuffer->updateData(indices.data());
}
