#pragma once

#include "Buffer.h"
#include "Device.h"
#include <glm/glm.hpp>
#include "DescriptorPool.h"
#include <array>
#include "GraphicsPipeline.h"
#include "RenderPass.h"
#include "MeshBase.h"

class Model
{
public:
	virtual ~Model();

	glm::mat4 getModelMatrix();

	void setModelMatrix(glm::mat4 matrix);

	uint32_t getBufferCount() const;

	uint32_t getTextureCount() const;

	uint32_t getMeshCount() const;

	void initDescriptorSets(DescriptorPool *pDescriptorPool);

	virtual void draw(VkCommandBuffer commandBuffer, std::vector<VkDescriptorSet> descriptorSets);

protected:
	Model(Device *pDevice);

	Device *pDevice;

	std::vector<MeshBase*> meshes;

	static VkDescriptorSetLayout modelMatrixDSLayout;

	// descriptor sets for each mesh
	std::vector<VkDescriptorSet> meshDescriptorSets;

	virtual VkDescriptorSetLayout& getMeshDSLayout() = 0;

	virtual GraphicsPipeline* getPipeline() = 0;

private:
	static uint32_t objectCount;

	glm::mat4 modelMatrix;

	Buffer *pModelMatrixBuffer;

	// descritpor set for mvp buffer
	VkDescriptorSet modelMatrixDescriptorSet;
};

