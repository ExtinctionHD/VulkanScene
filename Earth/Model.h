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

	void initSize(glm::vec3 minVertex, glm::vec3 maxVertex);

	void initBuffers(Device *pDevice);
};

