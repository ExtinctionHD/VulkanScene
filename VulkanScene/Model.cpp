#include "Model.h"
#include <stdexcept>

// public:

Model::~Model()
{
	for (auto material : materials)
    {
		delete material.second;
	}

	for(auto mesh : solidMeshes)
	{
		delete mesh;
	}

	for (auto mesh : transparentMeshes)
	{
		delete mesh;
	}

	delete transformationsBuffer;
}

uint32_t Model::getBufferCount() const 
{
	return uint32_t(1 + materials.size());
}

uint32_t Model::getTextureCount() const
{
	return uint32_t(Material::TEXTURES_ORDER.size() * materials.size());
}

uint32_t Model::getDescriptorSetCount() const
{
	return uint32_t(1 + materials.size());
}

uint32_t Model::getMeshCount() const
{
	return uint32_t(solidMeshes.size());
}

Transformation Model::getTransformation(uint32_t index)
{
	return { transformations[index] };
}

void Model::setTransformation(Transformation transformation, uint32_t index)
{
	transformations[index] = transformation.getMatrix();
	transformationsBuffer->updateData(&transformations[index], sizeof glm::mat4, index * sizeof glm::mat4);
}

GraphicsPipeline* Model::getPipeline(RenderPassType type) const
{
	return pipelines.at(type);
}

void Model::initDescriptorSets(DescriptorPool *descriptorPool)
{
	for (auto material : materials)
	{
		material.second->initDescriptorSet(descriptorPool);
	}
}

GraphicsPipeline * Model::createPipeline(
    RenderPassType type,
    RenderPass *renderPass,
    const std::vector<VkDescriptorSetLayout> &layouts,
	const std::vector<VkPushConstantRange> &pushConstantRanges,
    const std::vector<std::shared_ptr<ShaderModule>> &shaderModules)
{
	const std::vector<VkVertexInputBindingDescription> bindingDescriptions = {
		getVertexBindingDescription(0),
		getTransformationBindingDescription(1)
	};

	std::vector<VkVertexInputAttributeDescription> attributeDescriptions = getVertexAttributeDescriptions(0, 0);
	std::vector<VkVertexInputAttributeDescription> transformationAttributeDescriptions =
        getTransformationAttributeDescriptions(1, uint32_t(attributeDescriptions.size()));
	attributeDescriptions.insert(
        attributeDescriptions.end(),
        transformationAttributeDescriptions.begin(),
        transformationAttributeDescriptions.end());

    switch (type)
    {
    case DEPTH:
		return createDepthPipeline(
            renderPass,
            layouts,
            pushConstantRanges,
            shaderModules,
            bindingDescriptions,
            attributeDescriptions);
    case GEOMETRY:
		return createGeometryPipeline(
            renderPass,
            layouts,
            pushConstantRanges,
            shaderModules,
            bindingDescriptions,
            attributeDescriptions);
    case FINAL:
		return createFinalPipeline(
            renderPass,
            layouts,
            pushConstantRanges,
            shaderModules,
            bindingDescriptions,
            attributeDescriptions);
    default: 
        throw std::invalid_argument("No pipeline for this render pass type");
    }
}

void Model::setPipeline(RenderPassType type, GraphicsPipeline *pipeline)
{
	pipelines.insert({ type, pipeline });
}

void Model::setStaticPipeline(RenderPassType type, GraphicsPipeline *pipeline)
{
	staticPipelines.insert({ type, pipeline });
}

void Model::renderDepth(VkCommandBuffer commandBuffer, const std::vector<VkDescriptorSet> &descriptorSets, uint32_t renderIndex) const
{
    const std::vector<VkPushConstantRange> pushConstantRanges{
		{ VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(uint32_t) }
	};
    const std::vector<const void *> pushConstantData{
		&renderIndex
	};

	renderMeshes(commandBuffer, DEPTH, descriptorSets, pushConstantRanges, pushConstantData, solidMeshes);
	renderMeshes(commandBuffer, DEPTH, descriptorSets, pushConstantRanges, pushConstantData, transparentMeshes);
}

void Model::renderGeometry(VkCommandBuffer commandBuffer, const std::vector<VkDescriptorSet> &descriptorSets) const
{
	renderMeshes(commandBuffer, GEOMETRY, descriptorSets, {}, {}, solidMeshes);
}

void Model::renderFinal(VkCommandBuffer commandBuffer, const std::vector<VkDescriptorSet> &descriptorSets) const
{
	renderMeshes(commandBuffer, FINAL, descriptorSets, {}, {}, transparentMeshes);
}

void Model::renderFullscreenQuad(
    VkCommandBuffer commandBuffer,
    RenderPassType type,
    const std::vector<VkDescriptorSet> &descriptorSets)
{
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, staticPipelines.at(type)->get());

	vkCmdBindDescriptorSets(
		commandBuffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		staticPipelines.at(type)->getLayout(),
		0,
		uint32_t(descriptorSets.size()),
		descriptorSets.data(),
		0,
		nullptr);

	vkCmdDraw(commandBuffer, 3, 1, 0, 0);
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

Model::Model(Device *device, uint32_t count)
{
	this->device = device;

	transformationsBuffer = new Buffer(device, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, count * sizeof glm::mat4);
	transformations.resize(count, glm::mat4(1.0f));
	transformationsBuffer->updateData(transformations.data(), count * sizeof glm::mat4, 0);
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

std::vector<VkVertexInputAttributeDescription> Model::getTransformationAttributeDescriptions(
    uint32_t binding,
    uint32_t locationOffset)
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
    RenderPass *renderPass,
    std::vector<VkDescriptorSetLayout> layouts,
	const std::vector<VkPushConstantRange> &pushConstantRanges,
    const std::vector<std::shared_ptr<ShaderModule>> &shaderModules,
    const std::vector<VkVertexInputBindingDescription> &bindingDescriptions,
    const std::vector<VkVertexInputAttributeDescription> &attributeDescriptions)
{
	layouts.push_back(Material::getDsLayout());

    const auto pipeline = new GraphicsPipeline(
		device,
		renderPass,
		layouts,
        pushConstantRanges,
	    shaderModules,
		bindingDescriptions,
		attributeDescriptions,
        false);

	setPipeline(DEPTH, pipeline);

	return pipeline;
}

GraphicsPipeline* Model::createGeometryPipeline(
    RenderPass *renderPass,
    std::vector<VkDescriptorSetLayout> layouts,
	const std::vector<VkPushConstantRange> &pushConstantRanges,
    const std::vector<std::shared_ptr<ShaderModule>> &shaderModules,
    const std::vector<VkVertexInputBindingDescription> &bindingDescriptions,
    const std::vector<VkVertexInputAttributeDescription> &attributeDescriptions)
{
	layouts.push_back(Material::getDsLayout());

    const auto pipeline = new GraphicsPipeline(
		device,
		renderPass,
		layouts,
		pushConstantRanges,
	    shaderModules,
		bindingDescriptions,
		attributeDescriptions,
        false);

	setPipeline(GEOMETRY, pipeline);

	return pipeline;
}

GraphicsPipeline* Model::createFinalPipeline(
    RenderPass *renderPass,
    std::vector<VkDescriptorSetLayout> layouts,
	const std::vector<VkPushConstantRange> &pushConstantRanges,
    const std::vector<std::shared_ptr<ShaderModule>> &shaderModules,
    const std::vector<VkVertexInputBindingDescription> &bindingDescriptions,
    const std::vector<VkVertexInputAttributeDescription> &attributeDescriptions)
{
	layouts.push_back(Material::getDsLayout());

    const auto pipeline = new GraphicsPipeline(
		device,
		renderPass,
		layouts,
		pushConstantRanges,
	    shaderModules,
		bindingDescriptions,
		attributeDescriptions,
        true);

	setPipeline(FINAL, pipeline);

	return pipeline;
}

void Model::renderMeshes(
    VkCommandBuffer commandBuffer,
    RenderPassType type,
    const std::vector<VkDescriptorSet> &descriptorSets,
    const std::vector<VkPushConstantRange> &pushConstantRanges,
    const std::vector<const void *> &pushConstantData,
    std::vector<MeshBase*> meshes) const
{
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.at(type)->get());


    for (uint32_t i = 0; i < pushConstantRanges.size(); i++)
    {
		vkCmdPushConstants(
            commandBuffer,
            pipelines.at(type)->getLayout(),
            pushConstantRanges[i].stageFlags,
            pushConstantRanges[i].offset,
            pushConstantRanges[i].size,
            pushConstantData[i]);
    }

	vkCmdBindDescriptorSets(
        commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipelines.at(type)->getLayout(),
        0,
        uint32_t(descriptorSets.size()),
        descriptorSets.data(),
        0,
        nullptr);

	VkBuffer buffer = transformationsBuffer->get();
	VkDeviceSize offset = 0;
	vkCmdBindVertexBuffers(commandBuffer, 1, 1, &buffer, &offset);

	for (auto &mesh : meshes)
	{
		VkDescriptorSet materialDescriptorSet = mesh->getMaterial()->getDescriptorSet();
		vkCmdBindDescriptorSets(
            commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelines.at(type)->getLayout(),
            uint32_t(descriptorSets.size()),
            1,
            &materialDescriptorSet,
            0,
            nullptr);

		mesh->render(commandBuffer, uint32_t(transformations.size()));
	}
}


