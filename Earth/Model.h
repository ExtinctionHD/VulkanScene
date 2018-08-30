#pragma once

#include <vector>
#include <iostream>
#include "Vertex.h"
#include <tiny_obj_loader.h>
#include "Buffer.h"
#include "Device.h"

// model vertices and vertex indies
class Model
{
public:
	Model(Device *pDevice, std::string filename);
	~Model();

	// model size in 3d
	VkExtent3D size;

	Buffer *pVertexBuffer;
	Buffer *pIndexBuffer;

	size_t getIndexCount();

private:
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	void initVectors(tinyobj::attrib_t attrib, std::vector<tinyobj::shape_t> shapes);

	void initBuffers(Device *pDevice);
};

