#pragma once

#include "Buffer.h"
#include "Device.h"
#include <glm/glm.hpp>
#include "DescriptorPool.h"
#include "GraphicsPipeline.h"
#include "RenderPass.h"
#include "MeshBase.h"
#include <map>

class Model
{
public:
	virtual ~Model();

	glm::mat4 getTransform();

	void setTransform(glm::mat4 matrix);

	uint32_t getBufferCount() const;

	uint32_t getTextureCount() const;

	uint32_t getMeshCount() const;

	void initDescriptorSets(DescriptorPool *pDescriptorPool);

	GraphicsPipeline* createPipeline(std::vector<VkDescriptorSetLayout> layouts, RenderPass * pRenderPass, std::vector<ShaderModule*> shaderModules);

	void setPipeline(GraphicsPipeline *pPipeline);

	void draw(VkCommandBuffer commandBuffer, std::vector<VkDescriptorSet> descriptorSets);

protected:
	Model(Device *pDevice);

	Device *pDevice;

	std::vector<MeshBase*> meshes;

	std::map<uint32_t, Material*> materials;

	virtual VkVertexInputBindingDescription getVertexInputBindingDescription(uint32_t inputBinding) = 0;

	virtual std::vector<VkVertexInputAttributeDescription> getVertexInputAttributeDescriptions(uint32_t inputBinding) = 0;

private:
	static uint32_t objectCount;

	GraphicsPipeline *pPipeline{};

	glm::mat4 transform{};

	Buffer *pTransformBuffer;

	// descritpor set for mvp buffer
	VkDescriptorSet transformDescriptorSet{};

	static VkDescriptorSetLayout transformDSLayout;
};

