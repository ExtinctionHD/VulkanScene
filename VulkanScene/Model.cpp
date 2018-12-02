#include "Model.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>

// public:

Model::~Model()
{
	objectCount--;

	if (objectCount == 0 && transformDsLayout != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorSetLayout(pDevice->device, transformDsLayout, nullptr);
		transformDsLayout = VK_NULL_HANDLE;
	}

	// cleanup materials
	for (auto it = materials.begin(); it != materials.end(); ++it)
	{
		delete((*it).second);
	}

	// cleanup meshes
	for (auto it = solidMeshes.begin(); it != solidMeshes.end(); ++it)
	{
		delete(*it);
	}

	delete(pTransformBuffer);
}

glm::mat4 Model::getTransform() const
{
	return transform;
}

void Model::setTransform(glm::mat4 matrix)
{
	transform = matrix;
	pTransformBuffer->updateData(&transform, sizeof(transform), 0);
}

void Model::rotate(glm::vec3 axis, float angle)
{
	setTransform(glm::rotate(getTransform(), glm::radians(angle), axis));
}

void Model::rotateAxisX(float angle)
{
	setTransform(glm::rotate(getTransform(), glm::radians(angle), glm::vec3(1.0f, 0.0f, 0.0f)));
}

void Model::rotateAxisY(float angle)
{
	setTransform(glm::rotate(getTransform(), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f)));
}

void Model::rotateAxisZ(float angle)
{
	setTransform(glm::rotate(getTransform(), glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f)));
}

void Model::move(glm::vec3 distance)
{
	setTransform(translate(getTransform(), distance));
}

void Model::scale(glm::vec3 scale)
{
	setTransform(glm::scale(getTransform(), scale));
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

VkDescriptorSetLayout Model::getTransformDsLayout()
{
	return transformDsLayout;
}

void Model::initDescriptorSets(DescriptorPool * pDescriptorPool)
{
    if (transformDsLayout == VK_NULL_HANDLE)
    {
		transformDsLayout = pDescriptorPool->createDescriptorSetLayout({ VK_SHADER_STAGE_VERTEX_BIT }, {});
    }

	transformDescriptorSet = pDescriptorPool->getDescriptorSet(
		{ pTransformBuffer },
		{},
		transformDsLayout
	);

	for (auto material : materials)
	{
		material.second->initDescriptorSet(pDescriptorPool);
	}
}

GraphicsPipeline * Model::createPipeline(
    const std::vector<VkDescriptorSetLayout> &layouts,
    RenderPassType type,
    RenderPass *pRenderPass,
    const std::vector<ShaderModule *> &shaderModules
)
{
    switch (type)
    {
    case DEPTH:
		return createDepthPipeline(layouts, pRenderPass, shaderModules);
    case GEOMETRY:
		return createGeometryPipeline(layouts, pRenderPass, shaderModules);
    case FINAL:
		return createFinalPipeline(layouts, pRenderPass, shaderModules);
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

void Model::renderDepth(VkCommandBuffer commandBuffer, std::vector<VkDescriptorSet> descriptorSets) const
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

Model::Model(Device *pDevice)
{
	this->pDevice = pDevice;

	pTransformBuffer = new Buffer(pDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(transform));
	setTransform(glm::mat4(1.0f));

	objectCount++;
}

// private:

uint32_t Model::objectCount = 0;

VkDescriptorSetLayout Model::transformDsLayout = VK_NULL_HANDLE;

std::unordered_map<RenderPassType, GraphicsPipeline*> Model::staticPipelines;

GraphicsPipeline* Model::createDepthPipeline(
	std::vector<VkDescriptorSetLayout> layouts,
	RenderPass *pRenderPass,
	std::vector<ShaderModule*> shaderModules
)
{
	layouts.push_back(transformDsLayout);
	layouts.push_back(Material::getDsLayout());

	const uint32_t inputBinding = 0;

	GraphicsPipeline *pPipeline = new GraphicsPipeline(
		pDevice,
		layouts,
		pRenderPass,
	    shaderModules,
		{ getVertexInputBindingDescription(inputBinding) },
		getVertexInputAttributeDescriptions(inputBinding),
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
	std::vector<ShaderModule*> shaderModules
)
{
	layouts.push_back(transformDsLayout);
	layouts.push_back(Material::getDsLayout());

	const uint32_t inputBinding = 0;

	GraphicsPipeline *pPipeline = new GraphicsPipeline(
		pDevice,
		layouts,
		pRenderPass,
	    shaderModules,
		{ getVertexInputBindingDescription(inputBinding) },
		getVertexInputAttributeDescriptions(inputBinding),
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
	std::vector<ShaderModule*> shaderModules
)
{
	layouts.push_back(transformDsLayout);
	layouts.push_back(Material::getDsLayout());

	const uint32_t inputBinding = 0;

	GraphicsPipeline *pPipeline = new GraphicsPipeline(
		pDevice,
		layouts,
		pRenderPass,
	    shaderModules,
		{ getVertexInputBindingDescription(inputBinding) },
		getVertexInputAttributeDescriptions(inputBinding),
		pRenderPass->getSampleCount(),
		pRenderPass->getColorAttachmentCount(),
        VK_TRUE
	);
	setPipeline(FINAL, pPipeline);

	return pPipeline;
}

void Model::renderMeshes(VkCommandBuffer commandBuffer, std::vector<VkDescriptorSet> descriptorSets, RenderPassType type, std::vector<MeshBase*> meshes) const
{
	descriptorSets.push_back(transformDescriptorSet);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.at(type)->pipeline);

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.at(type)->layout, 0, descriptorSets.size(), descriptorSets.data(), 0, nullptr);

	for (auto &mesh : meshes)
	{
		VkDescriptorSet materialDescriptorSet = mesh->pMaterial->getDescriptorSet();
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.at(type)->layout, descriptorSets.size(), 1, &materialDescriptorSet, 0, nullptr);

		mesh->draw(commandBuffer);
	}
}


