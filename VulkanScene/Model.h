#pragma once

#include "Buffer.h"
#include "Device.h"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
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

	glm::mat4 getTransform() const;

	void setTransform(glm::mat4 matrix);

	void rotate(glm::vec3 axis, float angle);

	void rotateAxisX(float angle);

	void rotateAxisY(float angle);

	void rotateAxisZ(float angle);

	void move(glm::vec3 distance);

	void scale(glm::vec3 scale);

	uint32_t getBufferCount() const;

	uint32_t getTextureCount() const;

	uint32_t getDescriptorSetCount() const;

	uint32_t getMeshCount() const;

	GraphicsPipeline* getPipeline(RenderPassType type) const;

	static VkDescriptorSetLayout getTransformDsLayout();

	void initDescriptorSets(DescriptorPool *pDescriptorPool);

	GraphicsPipeline* createPipeline(
	    const std::vector<VkDescriptorSetLayout> &layouts,
		RenderPassType type,
		RenderPass *pRenderPass,
	    const std::vector<ShaderModule*> &shaderModules
	);

	void setPipeline(RenderPassType type, GraphicsPipeline *pPipeline);

	static void setStaticPipeline(RenderPassType type, GraphicsPipeline *pPipeline);

	void renderDepth(VkCommandBuffer commandBuffer, std::vector<VkDescriptorSet> descriptorSets) const;

	void renderGeometry(VkCommandBuffer commandBuffer, std::vector<VkDescriptorSet> descriptorSets) const;

	static void renderFullscreenQuad(VkCommandBuffer commandBuffer, std::vector<VkDescriptorSet> descriptorSets, RenderPassType type);

	void renderFinal(VkCommandBuffer commandBuffer, std::vector<VkDescriptorSet> descriptorSets) const;

	void optimizeMemory();

protected:
	Model(Device *pDevice);

	Device *pDevice;

	std::vector<MeshBase*> solidMeshes;

	std::vector<MeshBase*> transparentMeshes;

	std::map<uint32_t, Material*> materials;

	virtual VkVertexInputBindingDescription getVertexInputBindingDescription(uint32_t inputBinding) = 0;

	virtual std::vector<VkVertexInputAttributeDescription> getVertexInputAttributeDescriptions(uint32_t inputBinding) = 0;

private:
	static uint32_t objectCount;

	std::unordered_map<RenderPassType, GraphicsPipeline*> pipelines;

	glm::mat4 transform{};

	Buffer *pTransformBuffer;

	// descriptor set for mvp buffer
	VkDescriptorSet transformDescriptorSet{};

	static VkDescriptorSetLayout transformDsLayout;

	static std::unordered_map<RenderPassType, GraphicsPipeline*> staticPipelines;

	GraphicsPipeline* createDepthPipeline(std::vector<VkDescriptorSetLayout> layouts, RenderPass * pRenderPass, std::vector<ShaderModule*> shaderModules);

	GraphicsPipeline* createGeometryPipeline(std::vector<VkDescriptorSetLayout> layouts, RenderPass * pRenderPass, std::vector<ShaderModule*> shaderModules);

	GraphicsPipeline* createFinalPipeline(std::vector<VkDescriptorSetLayout> layouts, RenderPass * pRenderPass, std::vector<ShaderModule*> shaderModules);

	void renderMeshes(VkCommandBuffer commandBuffer, std::vector<VkDescriptorSet> descriptorSets, RenderPassType type, std::vector<MeshBase*> meshes) const;
};

