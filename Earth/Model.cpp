#define TINYOBJLOADER_IMPLEMENTATION
#include "File.h"
#include "Logger.h"

#include "Model.h"

// public:

Model::Model(std::string filename)
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

	initModel(attrib, shapes);
}

Model::~Model()
{
}

// private:

void Model::initModel(tinyobj::attrib_t attrib, std::vector<tinyobj::shape_t> shapes)
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
