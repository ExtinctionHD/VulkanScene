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
#include "Transformation.h"

class Model
{
public:
	virtual ~Model();

	uint32_t getBufferCount() const;

	uint32_t getTextureCount() const;

	uint32_t getDescriptorSetCount() const;

	uint32_t getMeshCount() const;

	Transformation getTransformation(uint32_t index);

	void setTransformation(Transformation transformation, uint32_t index);

	GraphicsPipeline* getPipeline(RenderPassType type) const;

	void initDescriptorSets(DescriptorPool *descriptorPool);

	GraphicsPipeline* createPipeline(
        RenderPassType type,
        RenderPass *renderPass,
        const std::vector<VkDescriptorSetLayout> &layouts,
		const std::vector<VkPushConstantRange> &pushConstantRanges,
        const std::vector<std::shared_ptr<ShaderModule>> &shaderModules);

	void setPipeline(RenderPassType type, GraphicsPipeline *pipeline);

	static void setStaticPipeline(RenderPassType type, GraphicsPipeline *pipeline);

	void renderDepth(VkCommandBuffer commandBuffer, const std::vector<VkDescriptorSet> &descriptorSets, uint32_t renderIndex) const;

	void renderGeometry(VkCommandBuffer commandBuffer, const std::vector<VkDescriptorSet> &descriptorSets) const;

	void renderFinal(VkCommandBuffer commandBuffer, const std::vector<VkDescriptorSet> &descriptorSets) const;

	static void renderFullscreenQuad(
        VkCommandBuffer commandBuffer,
        RenderPassType type,
        const std::vector<VkDescriptorSet> &descriptorSets);

	void optimizeMemory();

protected:
	Model(Device *device, uint32_t count);

	Device *device;

	std::vector<MeshBase*> solidMeshes;

	std::vector<MeshBase*> transparentMeshes;

	std::map<uint32_t, Material*> materials;

	virtual VkVertexInputBindingDescription getVertexBindingDescription(uint32_t binding) = 0;

	virtual std::vector<VkVertexInputAttributeDescription> getVertexAttributeDescriptions(
        uint32_t binding,
        uint32_t locationOffset) = 0;

private:
	std::unordered_map<RenderPassType, GraphicsPipeline*> pipelines;

	std::vector<glm::mat4> transformations;

	Buffer *transformationsBuffer;

	static std::unordered_map<RenderPassType, GraphicsPipeline*> staticPipelines;

	static VkVertexInputBindingDescription getTransformationBindingDescription(uint32_t inputBinding);

	static std::vector<VkVertexInputAttributeDescription> getTransformationAttributeDescriptions(
        uint32_t binding,
        uint32_t locationOffset);

	GraphicsPipeline* createDepthPipeline(
        RenderPass *renderPass,
        std::vector<VkDescriptorSetLayout> layouts,
		const std::vector<VkPushConstantRange> &pushConstantRanges,
        const std::vector<std::shared_ptr<ShaderModule>> &shaderModules,
        const std::vector<VkVertexInputBindingDescription> &bindingDescriptions,
        const std::vector<VkVertexInputAttributeDescription> &attributeDescriptions);

	GraphicsPipeline* createGeometryPipeline(
        RenderPass *renderPass,
        std::vector<VkDescriptorSetLayout> layouts,
		const std::vector<VkPushConstantRange> &pushConstantRanges,
        const std::vector<std::shared_ptr<ShaderModule>> &shaderModules,
        const std::vector<VkVertexInputBindingDescription> &bindingDescriptions,
        const std::vector<VkVertexInputAttributeDescription> &attributeDescriptions);

	GraphicsPipeline* createFinalPipeline(
        RenderPass *renderPass,
        std::vector<VkDescriptorSetLayout> layouts,
		const std::vector<VkPushConstantRange> &pushConstantRanges,
        const std::vector<std::shared_ptr<ShaderModule>> &shaderModules,
        const std::vector<VkVertexInputBindingDescription> &bindingDescriptions,
        const std::vector<VkVertexInputAttributeDescription> &attributeDescriptions);

	void renderMeshes(
        VkCommandBuffer commandBuffer,
        RenderPassType type,
        const std::vector<VkDescriptorSet> &descriptorSets,
		const std::vector<VkPushConstantRange> &pushConstantRanges,
		const std::vector<const void *> &pushConstantData,
        std::vector<MeshBase*> meshes) const;
};

