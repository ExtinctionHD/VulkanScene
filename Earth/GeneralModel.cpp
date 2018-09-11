#define TINYOBJLOADER_IMPLEMENTATION
#include "File.h"
#include "Logger.h"

#include "GeneralModel.h"

// public:

GeneralModel::GeneralModel(Device *pDevice, std::string filename)
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

// private:

void GeneralModel::normilize()
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

void GeneralModel::initVectors(tinyobj::attrib_t attrib, std::vector<tinyobj::shape_t> shapes)
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

			findMaxMin(vertex);

			vertices.push_back(vertex);
			indices.push_back(indices.size());
		}
	}

	
	initNormals();	// initialize normal attribute of each vertex
	initTangents();	// initialize tangent attribute of each vertex
}

void GeneralModel::findMaxMin(Vertex vertex)
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

void GeneralModel::initNormals()
{
	for (uint32_t i = 0; i < indices.size(); i += 3)
	{
		uint32_t Index0 = indices[i];
		uint32_t Index1 = indices[i + 1];
		uint32_t Index2 = indices[i + 2];

		glm::vec3 v1 = vertices[Index1].pos - vertices[Index0].pos;
		glm::vec3 v2 = vertices[Index2].pos - vertices[Index0].pos;
		glm::vec3 normal = glm::cross(v1, v2);
		normal = glm::normalize(normal);

		vertices[Index0].normal += normal;
		vertices[Index1].normal += normal;
		vertices[Index2].normal += normal;
	}

	for (uint32_t i = 0; i < vertices.size(); i++) {
		vertices[i].normal = glm::normalize(vertices[i].normal);
	}
}

void GeneralModel::initTangents()
{
	for (uint32_t i = 0; i < indices.size(); i += 3) 
	{
		Vertex& v0 = vertices[indices[i]];
		Vertex& v1 = vertices[indices[i + 1]];
		Vertex& v2 = vertices[indices[i + 2]];

		glm::vec3 edge1 = v1.pos - v0.pos;
		glm::vec3 edge2 = v2.pos - v0.pos;

		float deltaU1 = v1.tex.x - v0.tex.x;
		float deltaV1 = v1.tex.y - v0.tex.y;
		float deltaU2 = v2.tex.x - v0.tex.x;
		float deltaV2 = v2.tex.y - v0.tex.y;

		float f = 1.0f / (deltaU1 * deltaV2 - deltaU2 * deltaV1);

		glm::vec3 tangent;

		tangent.x = f * (deltaV2 * edge1.x - deltaV1 * edge2.x);
		tangent.y = f * (deltaV2 * edge1.y - deltaV1 * edge2.y);
		tangent.z = f * (deltaV2 * edge1.z - deltaV1 * edge2.z);

		v0.tangent += tangent;
		v1.tangent += tangent;
		v2.tangent += tangent;
	}

	for (uint32_t i = 0; i < vertices.size(); i++) 
	{
		vertices[i].tangent = glm::normalize(vertices[i].tangent);
	}
}

void GeneralModel::initSize(glm::vec3 minVertex, glm::vec3 maxVertex)
{
	size.x = maxVertex.x - minVertex.x;
	size.y = maxVertex.y - minVertex.y;
	size.z = maxVertex.z - minVertex.z;
}

void GeneralModel::initBuffers(Device *pDevice)
{
	VkDeviceSize size = vertices.size() * sizeof(vertices[0]);
	pVertexBuffer = new Buffer(pDevice, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_SHADER_STAGE_ALL, size);
	pVertexBuffer->updateData(vertices.data());

	size = indices.size() * sizeof(indices[0]);
	pIndexBuffer = new Buffer(pDevice, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_SHADER_STAGE_ALL, size);
	pIndexBuffer->updateData(indices.data());
}
