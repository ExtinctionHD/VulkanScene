#pragma once

#include <vector>
#include <iostream>
#include "Vertex.h"
#include <tiny_obj_loader.h>
#include <glm/glm.hpp>
#include "Buffer.h"
#include "Device.h"
#include "ModelBase.h"

// model vertices and vertex indies
class GeneralModel : public ModelBase
{
public:
	GeneralModel(Device *pDevice, std::string filename);
	~GeneralModel() {}

	// model size in 3d
	glm::vec3 size;

	// vertices with extreme values of x, y, z
	glm::vec3 minVertex = glm::vec3(max, max, max);
	glm::vec3 maxVertex = glm::vec3(min, min, min);

	// translate model center in 0, 0, 0
	void normilize();

private:
	const float min = std::numeric_limits<float>::min();
	const float max = std::numeric_limits<float>::max();

	std::vector<Vertex> vertices;

	// initialize vertex array and extreme values
	void initVectors(tinyobj::attrib_t attrib, std::vector<tinyobj::shape_t> shapes);

	void findMaxMin(Vertex vertex);

	void initNormals();

	void initTangents();

	void initSize(glm::vec3 minVertex, glm::vec3 maxVertex);

	void initBuffers(Device *pDevice);
};

