#include "Model.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>

// public:

Model::~Model()
{
	// cleanup materials
	for (auto it = materials.begin(); it != materials.end(); ++it)
	{
		delete((*it).second);
	}

	// cleanup meshes
	for(auto mesh : solidMeshes)
	{
		delete mesh;
	}
	for (auto mesh : transparentMeshes)
	{
		delete mesh;
	}

	delete(pTransformationsBuffer);
}

glm::mat4 Model::getTransformation(uint32_t index) const
{
	return transformations[index];
}

void Model::setTransformations(glm::mat4 matrix)
{
	const uint32_t count = transformations.size();
	transformations = std::vector<glm::mat4>(count, matrix);
	pTransformationsBuffer->updateData(transformations.data(), count * sizeof glm::mat4, 0);
}

void Model::setTransformation(glm::mat4 matrix, uint32_t index)
{
	transformations[index] = matrix;
	pTransformationsBuffer->updateData(&transformations[index], sizeof glm::mat4, index * sizeof glm::mat4);
}

void Model::rotate(glm::vec3 axis, float angle, uint32_t index)
{
	setTransformation(glm::rotate(getTransformation(index), glm::radians(angle), axis), index);
}

void Model::rotateAxisX(float angle, uint32_t index)
{
	setTransformation(glm::rotate(getTransformation(index), glm::radians(angle), glm::vec3(1.0f, 0.0f, 0.0f)), index);
}

void Model::rotateAxisY(float angle, uint32_t index)
{
	setTransformation(glm::rotate(getTransformation(index), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f)), index);
}

void Model::rotateAxisZ(float angle, uint32_t index)
{
	setTransformation(glm::rotate(getTransformation(index), glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f)), index);
}

void Model::move(glm::vec3 distance, uint32_t index)
{
	setTransformation(translate(getTransformation(index), distance), index);
}

void Model::scale(glm::vec3 scale, uint32_t index)
{
	setTransformation(glm::scale(getTransformation(index), scale), index);
}

uint32_t Model::getBufferCount() const 
{
	return 1 + materials.size();
}

uint32_t Model::getTextureCount() const
{
	return Material::TEXTURES_ORDER.size() * materials.size();
}

uint32_t Model::getDescriptorSetCount() const
{
	return 1 + materials.size();
}

uint32_t Model::getMeshCount() const
{
	return solidMeshes.size();
}

GraphicsPipeline * Model::getPipeline(RenderPassType type) const
{
	return pipelines.at(type);
}

void Model::initDescriptorSets(DescriptorPool * pDescriptorPool)
{
	for (auto material : materials)
	{
		material.second->initDescriptorSet(pDescriptorPool);
	}
}

GraphicsPipeline * Model::createPipeline(
    const std::vector<VkDescriptorSetLayout> &layouts,
    RenderPassType type,
    RenderPass *pRenderPass,
    const std::vector<std::shared_ptr<ShaderModule>> &shaderModules
)
{
	const std::vector<VkVertexInputBindingDescription> bindingDescriptions = {
		getVertexBindingDescription(0),
		getTransformationBindingDescription(1)
	};

	std::vector<VkVertexInputAttributeDescription> attributeDescriptions = getVertexAttributeDescriptions(0, 0);
	std::vector<VkVertexInputAttributeDescription> transformationAttributeDescriptions = getTransformationAttributeDescriptions(1, attributeDescriptions.size());
	attributeDescriptions.insert(attributeDescriptions.end(), transformationAttributeDescriptions.begin(), transformationAttributeDescriptions.end());

    switch (type)
    {
    case DEPTH:
		return createDepthPipeline(layouts, pRenderPass, shaderModules, bindingDescriptions, attributeDescriptions);
    case GEOMETRY:
		return createGeometryPipeline(layouts, pRenderPass, shaderModules, bindingDescriptions, attributeDescriptions);
    case FINAL:
		return createFinalPipeline(layouts, pRenderPass, shaderModules, bindingDescriptions, attributeDescriptions);
    default: 
        throw std::invalid_argument("No pipeline for this render pass type");
    }
}

void Model::setPipeline(RenderPassType type, GraphicsPipeline * pPipeline)
{
	pipelines.insert({ type, pPipeline });
}

void Model::setStaticPipeline(RenderPassType type, GraphicsPipeline *pPipeline)
{
	staticPipelines.insert({ type, pPipeline });
}

void Model::renderDepth(VkCommandBuffer commandBuffer, const std::vector<VkDescriptorSet>& descriptorSets) const
{
	renderMeshes(commandBuffer, descriptorSets, DEPTH, solidMeshes);
	renderMeshes(commandBuffer, descriptorSets, DEPTH, transparentMeshes);
}

void Model::renderGeometry(VkCommandBuffer commandBuffer, std::vector<VkDescriptorSet> descriptorSets) const
{
	renderMeshes(commandBuffer, descriptorSets, GEOMETRY, solidMeshes);
}

void Model::renderFullscreenQuad(VkCommandBuffer commandBuffer, std::vector<VkDescriptorSet> descriptorSets, RenderPassType type)
{
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, staticPipelines.at(type)->pipeline);

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, staticPipelines.at(type)->layout, 0, descriptorSets.size(), descriptorSets.data(), 0, nullptr);

	vkCmdDraw(commandBuffer, 3, 1, 0, 0);
}

void Model::renderFinal(VkCommandBuffer commandBuffer, std::vector<VkDescriptorSet> descriptorSets) const
{
	renderMeshes(commandBuffer, descriptorSets, FINAL, transparentMeshes);
}

void Model::optimizeMemory()
{
    for (auto mesh : solidMeshes)
    {
		mesh->clearHostIndices();
		mesh->clearHostVertices();
    }

	for (auto mesh : transparentMeshes)
	{
		mesh->clearHostIndices();
		mesh->clearHostVertices();
	}
}

// protected:

Model::Model(Device *pDevice, uint32_t count)
{
	this->pDevice = pDevice;
	pTransformationsBuffer = new Buffer(pDevice, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, count * sizeof glm::mat4);

	transformations.resize(count);
	setTransformations(glm::mat4(1.0f));
}

// private:

std::unordered_map<RenderPassType, GraphicsPipeline*> Model::staticPipelines;

VkVertexInputBindingDescription Model::getTransformationBindingDescription(uint32_t inputBinding)
{
	return VkVertexInputBindingDescription{
		inputBinding,
		sizeof(glm::mat4),
		VK_VERTEX_INPUT_RATE_INSTANCE
	};
}

std::vector<VkVertexInputAttributeDescription> Model::getTransformationAttributeDescriptions(uint32_t binding, uint32_t locationOffset)
{
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

	for (int i = 0; i < sizeof glm::mat4 / sizeof glm::vec4; i++)
	{
		attributeDescriptions.push_back({
			locationOffset + i,
			binding,
			VK_FORMAT_R32G32B32A32_SFLOAT,
			i * sizeof glm::vec4
		});
	}

	return attributeDescriptions;
}

GraphicsPipeline* Model::createDepthPipeline(
	std::vector<VkDescriptorSetLayout> layouts,
	RenderPass *pRenderPass,
	std::vector<std::shared_ptr<ShaderModule>> shaderModules,
	std::vector<VkVertexInputBindingDescription> bindingDescriptions,
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions
)
{
	layouts.push_back(Material::getDsLayout());

	GraphicsPipeline *pPipeline = new GraphicsPipeline(
		pDevice,
		layouts,
		pRenderPass,
	    shaderModules,
		bindingDescriptions,
		attributeDescriptions,
		pRenderPass->getSampleCount(),
        pRenderPass->getColorAttachmentCount(),
        VK_FALSE
	);
	setPipeline(DEPTH, pPipeline);

	return pPipeline;
}

GraphicsPipeline* Model::createGeometryPipeline(
	std::vector<VkDescriptorSetLayout> layouts,
	RenderPass *pRenderPass,
	std::vector<std::shared_ptr<ShaderModule>> shaderModules,
	std::vector<VkVertexInputBindingDescription> bindingDescriptions,
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions
)
{
	layouts.push_back(Material::getDsLayout());

	GraphicsPipeline *pPipeline = new GraphicsPipeline(
		pDevice,
		layouts,
		pRenderPass,
	    shaderModules,
		bindingDescriptions,
		attributeDescriptions,
		pRenderPass->getSampleCount(),
        pRenderPass->getColorAttachmentCount(),
        VK_FALSE
	);
	setPipeline(GEOMETRY, pPipeline);

	return pPipeline;
}

GraphicsPipeline* Model::createFinalPipeline(
	std::vector<VkDescriptorSetLayout> layouts,
	RenderPass * pRenderPass,
	std::vector<std::shared_ptr<ShaderModule>> shaderModules,
	std::vector<VkVertexInputBindingDescription> bindingDescriptions,
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions
)
{
	layouts.push_back(Material::getDsLayout());

	GraphicsPipeline *pPipeline = new GraphicsPipeline(
		pDevice,
		layouts,
		pRenderPass,
	    shaderModules,
		bindingDescriptions,
		attributeDescriptions,
		pRenderPass->getSampleCount(),
		pRenderPass->getColorAttachmentCount(),
        VK_TRUE
	);
	setPipeline(FINAL, pPipeline);

	return pPipeline;
}

void Model::renderMeshes(VkCommandBuffer commandBuffer, std::vector<VkDescriptorSet> descriptorSets, RenderPassType type, std::vector<MeshBase*> meshes) const
{
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.at(type)->pipeline);

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.at(type)->layout, 0, descriptorSets.size(), descriptorSets.data(), 0, nullptr);

	VkBuffer transformationsBuffer = pTransformationsBuffer->getBuffer();
	VkDeviceSize offset = 0;
	vkCmdBindVertexBuffers(commandBuffer, 1, 1, &transformationsBuffer, &offset);

	for (auto &mesh : meshes)
	{
		VkDescriptorSet materialDescriptorSet = mesh->pMaterial->getDescriptorSet();
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.at(type)->layout, descriptorSets.size(), 1, &materialDescriptorSet, 0, nullptr);

		mesh->render(commandBuffer, transformations.size());
	}
}


