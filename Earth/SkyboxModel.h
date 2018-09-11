#pragma once

#include "Position.h"
#include "Buffer.h"
#include "ModelBase.h"
#include <vulkan/vulkan.h>
#include <vector>

class SkyboxModel : public ModelBase
{
public:
	SkyboxModel(Device *pDevice, std::vector<Position> vertices, std::vector<uint32_t> indices);
	~SkyboxModel() {}

private:
	std::vector<Position> vertices;

	void initBuffers(Device *pDevice);
};

