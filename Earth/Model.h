#pragma once

#include "MvpMatrices.h"
#include "Buffer.h"
#include "Device.h"
#include <glm/glm.hpp>

class Model
{
public:
	virtual ~Model();

	glm::mat4 getModelMatrix();

	void setModelMatrix(glm::mat4 model);

	void setViewMatrix(glm::mat4 view);

	void setProjectionMatrix(glm::mat4 proj);

	void setMvpMatrices(MvpMatrices mvp);

protected:
	Model(Device *pDevice);

	Device *pDevice;

	MvpMatrices mvp;

	Buffer *pMvpBuffer;
};

