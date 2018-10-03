#pragma once

#include "MvpMatrices.h"
#include "Buffer.h"
#include "Device.h"
#include <glm/glm.hpp>
#include "DescriptorPool.h"
#include <array>

class Model
{
public:
	virtual ~Model();

	glm::mat4 getModelMatrix();

	void setModelMatrix(glm::mat4 model);

	void setViewMatrix(glm::mat4 view);

	void setProjectionMatrix(glm::mat4 proj);

	void setMvpMatrices(MvpMatrices mvp);

	void initDescriptorSets(DescriptorPool *pDescriptorPool);

	// virtual void draw() = 0;

protected:
	Model(Device *pDevice);

	Device *pDevice;

	// descriptor sets for each mesh
	std::vector<VkDescriptorSet> meshDescriptorSets;

	void virtual initMeshDescriptorSets(DescriptorPool *pDescriptorPool) = 0;

private:
	static uint32_t objectCount;

	static VkDescriptorSetLayout mvpDSLayout;

	MvpMatrices mvp;

	Buffer *pMvpBuffer;

	// descritpor set for mvp buffer
	VkDescriptorSet mvpDescriptorSet;
};

