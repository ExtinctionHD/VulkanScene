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
	for (auto it = meshes.begin(); it != meshes.end(); ++it)
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

void Model::moveTo(glm::vec3 pos)
{
	glm::vec3 scale;
	glm::quat rotation;
	glm::vec3 translation;
	glm::vec3 skew;
	glm::vec4 perspective;

	decompose(getTransform(), scale, rotation, translation, skew, perspective);

	glm::mat4 transform = glm::scale(glm::mat4(1.0f), scale);
	transform = glm::rotate(transform, glm::angle(rotation), axis(rotation));
	transform = glm::translate(transform, pos);

	setTransform(transform);
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
	return meshes.size();
}

VkDescriptorSetLayout Model::getTransformDsLayout()
{
	return transformDsLayout;
}

void Model::initDescriptorSets(DescriptorPool * pDescriptorPool)
{
	transformDescriptorSet = pDescriptorPool->getDescriptorSet({ pTransformBuffer }, { }, transformDsLayout == VK_NULL_HANDLE, transformDsLayout);

	for (auto material : materials)
	{
		material.second->initDescriptorSet(pDescriptorPool);
	}
}

GraphicsPipeline * Model::createFinalPipeline(std::vector<VkDescriptorSetLayout> layouts, RenderPass * pRenderPass, std::vector<ShaderModule*> shaderModules)
{
	layouts.push_back(transformDsLayout);
	layouts.push_back(Material::getDsLayout());

	const uint32_t inputBinding = 0;

	pPipeline = new GraphicsPipeline(
		pDevice,
		layouts,
		pRenderPass,
		shaderModules,
		getVertexInputBindingDescription(inputBinding),
		getVertexInputAttributeDescriptions(inputBinding),
		pDevice->getSampleCount()
	);

	return pPipeline;
}

void Model::setPipeline(GraphicsPipeline * pPipeline)
{
	this->pPipeline = pPipeline;
}

void Model::drawDepth(
    VkCommandBuffer commandBuffer,
    std::vector<VkDescriptorSet> descriptorSets,
    GraphicsPipeline *pDepthPipeline
)
{
	descriptorSets.push_back(transformDescriptorSet);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pDepthPipeline->pipeline);

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pDepthPipeline->layout, 0, descriptorSets.size(), descriptorSets.data(), 0, nullptr);

	for (auto &mesh : meshes)
	{
		mesh->draw(commandBuffer);
	}
}


void Model::draw(VkCommandBuffer commandBuffer, std::vector<VkDescriptorSet> descriptorSets)
{
	descriptorSets.push_back(transformDescriptorSet);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pPipeline->pipeline);

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pPipeline->layout, 0, descriptorSets.size(), descriptorSets.data(), 0, nullptr);

	for (auto &mesh : meshes)
	{
		VkDescriptorSet materialDescriptorSet = mesh->pMaterial->getDescriptorSet();
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pPipeline->layout, descriptorSets.size(), 1, &materialDescriptorSet, 0, nullptr);

		mesh->draw(commandBuffer);
	}
}

// protected:

Model::Model(Device *pDevice)
{
	this->pDevice = pDevice;

	pTransformBuffer = new Buffer(pDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_SHADER_STAGE_VERTEX_BIT, sizeof(transform));
	setTransform(glm::mat4(1.0f));

	objectCount++;
}

// private:

uint32_t Model::objectCount = 0;

VkDescriptorSetLayout Model::transformDsLayout = VK_NULL_HANDLE;


