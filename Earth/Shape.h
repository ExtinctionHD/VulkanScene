#pragma once

#include "Position.h"
#include "Buffer.h"
#include "Mesh.h"
#include <vulkan/vulkan.h>
#include <vector>

class Shape : public Mesh
{
public:
	Shape(Device *pDevice, std::vector<Position> vertices, std::vector<uint32_t> indices);
	~Shape() {}

private:
	std::vector<Position> vertices;

	void initBuffers(Device *pDevice);
};

