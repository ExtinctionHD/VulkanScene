#pragma once

#include <vector>
#include <iostream>
#include "Vertex.h"
#include <tiny_obj_loader.h>
#include <glm/glm.hpp>
#include "Buffer.h"
#include "Device.h"

// model vertices and vertex indies
class Model
{
public:
	Model(Device *pDevice, std::string filename);
	~Model();

	// model size in 3d
	glm::vec3 size;

	Buffer *pVertexBuffer;
	Buffer *pIndexBuffer;

	// return number of indices
	size_t getIndexCount() const;

	// vertices with extreme values of x, y, z
	glm::vec3 minVertex = glm::vec3(max, max, max);
	glm::vec3 maxVertex = glm::vec3(min, min, min);

	// translate model center in 0, 0, 0
	void normilize();

private:
	const float min = std::numeric_limits<float>::min();
	const float max = std::numeric_limits<float>::max();

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	// initialize vertex array and extreme values
	void initVectors(tinyobj::attrib_t attrib, std::vector<tinyobj::shape_t> shapes);

	void initNormals()
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

	void initSize(glm::vec3 minVertex, glm::vec3 maxVertex);

	void initBuffers(Device *pDevice);
};

