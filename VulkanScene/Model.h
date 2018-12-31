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

	glm::mat4 getTransformation(uint32_t index) const;

	void setTransformations(glm::mat4 matrix);

	void setTransformation(glm::mat4 matrix, uint32_t index);

	void rotate(glm::vec3 axis, float angle, uint32_t index);

	void rotateAxisX(float angle, uint32_t index);

	void rotateAxisY(float angle, uint32_t index);

	void rotateAxisZ(float angle, uint32_t index);

	void move(glm::vec3 distance, uint32_t index);

	void scale(glm::vec3 scale, uint32_t index);

	uint32_t getBufferCount() const;

	uint32_t getTextureCount() const;

	uint32_t getDescriptorSetCount() const;

	uint32_t getMeshCount() const;

	GraphicsPipeline* getPipeline(RenderPassType type) const;

	void initDescriptorSets(DescriptorPool *pDescriptorPool);

	GraphicsPipeline* createPipeline(
	    const std::vector<VkDescriptorSetLayout> &layouts,
		RenderPassType type,
		RenderPass *pRenderPass,
	    const std::vector<std::shared_ptr<ShaderModule>> &shaderModules
	);

	void setPipeline(RenderPassType type, GraphicsPipeline *pPipeline);

	static void setStaticPipeline(RenderPassType type, GraphicsPipeline *pPipeline);

	void renderDepth(VkCommandBuffer commandBuffer, const std::vector<VkDescriptorSet>& descriptorSets) const;

	void renderGeometry(VkCommandBuffer commandBuffer, std::vector<VkDescriptorSet> descriptorSets) const;

	static void renderFullscreenQuad(VkCommandBuffer commandBuffer, std::vector<VkDescriptorSet> descriptorSets, RenderPassType type);

	void renderFinal(VkCommandBuffer commandBuffer, std::vector<VkDescriptorSet> descriptorSets) const;

	void optimizeMemory();

protected:
	Model(Device *pDevice, uint32_t count);

	Device *pDevice;

	std::vector<MeshBase*> solidMeshes;

	std::vector<MeshBase*> transparentMeshes;

	std::map<uint32_t, Material*> materials;

	virtual VkVertexInputBindingDescription getVertexBindingDescription(uint32_t binding) = 0;

	virtual std::vector<VkVertexInputAttributeDescription> getVertexAttributeDescriptions(uint32_t binding, uint32_t locationOffset) = 0;

private:
	std::unordered_map<RenderPassType, GraphicsPipeline*> pipelines;

	std::vector<glm::mat4> transformations{};

	Buffer *pTransformationsBuffer{};

	static std::unordered_map<RenderPassType, GraphicsPipeline*> staticPipelines;

	static VkVertexInputBindingDescription getTransformationBindingDescription(uint32_t inputBinding);

	static std::vector<VkVertexInputAttributeDescription> getTransformationAttributeDescriptions(uint32_t binding, uint32_t locationOffset);

	GraphicsPipeline* createDepthPipeline(
		std::vector<VkDescriptorSetLayout> layouts, 
		RenderPass * pRenderPass, 
		std::vector<std::shared_ptr<ShaderModule>> shaderModules,
		std::vector<VkVertexInputBindingDescription> bindingDescriptions,
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions
	);

	GraphicsPipeline* createGeometryPipeline(
		std::vector<VkDescriptorSetLayout> layouts,
		RenderPass * pRenderPass,
		std::vector<std::shared_ptr<ShaderModule>> shaderModules,
		std::vector<VkVertexInputBindingDescription> bindingDescriptions,
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions
	);

	GraphicsPipeline* createFinalPipeline(
		std::vector<VkDescriptorSetLayout> layouts,
		RenderPass * pRenderPass,
		std::vector<std::shared_ptr<ShaderModule>> shaderModules,
		std::vector<VkVertexInputBindingDescription> bindingDescriptions,
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions
	);

	void renderMeshes(VkCommandBuffer commandBuffer, std::vector<VkDescriptorSet> descriptorSets, RenderPassType type, std::vector<MeshBase*> meshes) const;
};

