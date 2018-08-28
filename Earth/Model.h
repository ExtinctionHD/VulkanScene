#pragma once

#include <vector>
#include <iostream>
#include "Vertex.h"
#include <tiny_obj_loader.h>

// model vertices and vertex indies
class Model
{
public:
	Model(std::string filename);
	~Model();

	VkExtent3D size;

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

private:
	void initModel(tinyobj::attrib_t attrib, std::vector<tinyobj::shape_t> shapes);
};

