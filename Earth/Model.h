#pragma once

#include "MvpMatrices.h"
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

	void setModelMatrix(glm::mat4 model);

	void setViewMatrix(glm::mat4 view);

	void setProjectionMatrix(glm::mat4 proj);

	void setMvpMatrices(MvpMatrices mvp);

	uint32_t getBufferCount() const;

	uint32_t getTextureCount() const;

	uint32_t getMeshCount() const;

	void initDescriptorSets(DescriptorPool *pDescriptorPool);

	virtual void draw(VkCommandBuffer commandBuffer, std::vector<VkDescriptorSet> descriptorSets);

protected:
	Model(Device *pDevice);

	Device *pDevice;

	std::vector<MeshBase*> meshes;

	static VkDescriptorSetLayout mvpDSLayout;

	// descriptor sets for each mesh
	std::vector<VkDescriptorSet> meshDescriptorSets;

	virtual VkDescriptorSetLayout& getMeshDSLayout() = 0;

	virtual GraphicsPipeline* getPipeline() = 0;

private:
	static uint32_t objectCount;

	MvpMatrices mvp;

	Buffer *pMvpBuffer;

	// descritpor set for mvp buffer
	VkDescriptorSet mvpDescriptorSet;
};

