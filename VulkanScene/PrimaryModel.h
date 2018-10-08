#pragma once

#include "Device.h"
#include <vector>
#include "PrimaryVertex.h"
#include "Buffer.h"

// model that consists of Primary vertices
class PrimaryModel
{
public:
	enum BaseModels
	{
		CUBE
	};

	PrimaryModel() {}
	PrimaryModel(Device *pDevice, std::vector<PrimaryVertex*> vertices, std::vector<uint32_t> indices);
	PrimaryModel(Device *pDevice, BaseModels base);
	~PrimaryModel();

	// vertices with extreme values of x, y, z
	glm::vec3 minVertex = glm::vec3(max, max, max);
	glm::vec3 maxVertex = glm::vec3(min, min, min);

	// model size in 3d
	glm::vec3 size;

	Buffer *pVertexBuffer;
	Buffer *pIndexBuffer;

	// return number of indices
	size_t getIndexCount() const;

	PrimaryModel& operator =(const PrimaryModel& model);

	// translate model center in 0, 0, 0
	void normilize();

protected:
	void findMaxMin(PrimaryVertex vertex);

	void initSize(glm::vec3 minVertex, glm::vec3 maxVertex);

	void initBuffers();

	virtual void updateVertexBuffer();

	std::vector<PrimaryVertex*> vertices;
	std::vector<uint32_t> indices;

	Device *pDevice;

private:
	const float min = std::numeric_limits<float>::min();
	const float max = std::numeric_limits<float>::max();
};

