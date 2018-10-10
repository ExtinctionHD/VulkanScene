#include "Model.h"

// public:

Model::~Model()
{
	objectCount--;

	if (objectCount == 0 && modelMatrixDSLayout != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorSetLayout(pDevice->device, modelMatrixDSLayout, nullptr);
		modelMatrixDSLayout = VK_NULL_HANDLE;
	}

	// cleanup materials
	for (auto it = meshes.begin(); it != meshes.end(); ++it)
	{
		delete(*it);
	}

	delete(pModelMatrixBuffer);
}

glm::mat4 Model::getModelMatrix()
{
	return modelMatrix;
}

void Model::setModelMatrix(glm::mat4 matrix)
{
	modelMatrix = matrix;
	pModelMatrixBuffer->updateData(&modelMatrix, sizeof(modelMatrix), 0);
}

uint32_t Model::getBufferCount() const 
{
	return 1 + meshes.size();
}

uint32_t Model::getTextureCount() const
{
	uint32_t textureCount = 0;

	for (MeshBase *pMesh : meshes)
	{
		textureCount += pMesh->getMaterialTextures().size();
	}

	return textureCount;
}

uint32_t Model::getMeshCount() const
{
	return meshes.size();
}

void Model::initDescriptorSets(DescriptorPool * pDescriptorPool)
{
	modelMatrixDescriptorSet = pDescriptorPool->getDescriptorSet({ pModelMatrixBuffer }, { }, Model::modelMatrixDSLayout == VK_NULL_HANDLE, Model::modelMatrixDSLayout);

	for (MeshBase *pMesh : meshes)
	{
		meshDescriptorSets.push_back(
			pDescriptorPool->getDescriptorSet(
				{ pMesh->getMaterialColorBuffer() },
				pMesh->getMaterialTextures(),
				getMeshDSLayout() == VK_NULL_HANDLE,
				getMeshDSLayout()
			)
		);
	}
}

void Model::draw(VkCommandBuffer commandBuffer, std::vector<VkDescriptorSet> descriptorSets)
{
	descriptorSets.push_back(modelMatrixDescriptorSet);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, getPipeline()->pipeline);

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, getPipeline()->layout, 0, descriptorSets.size(), descriptorSets.data(), 0, nullptr);

	for (int i = 0; i < meshes.size(); i++)
	{
		VkDescriptorSet meshDescriptorSet = meshDescriptorSets[i];
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, getPipeline()->layout, descriptorSets.size(), 1, &meshDescriptorSet, 0, nullptr);

		VkBuffer vertexBuffer = meshes[i]->getVertexBuffer();
		VkDeviceSize offset = 0;
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer, &offset);

		VkBuffer indexBuffer = meshes[i]->getIndexBuffer();
		vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

		uint32_t indexCount = meshes[i]->getIndexCount();
		vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
	}
}

// protected:

Model::Model(Device *pDevice)
{
	this->pDevice = pDevice;

	pModelMatrixBuffer = new Buffer(pDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_SHADER_STAGE_VERTEX_BIT, sizeof(modelMatrix));
	setModelMatrix(glm::mat4(1.0f));

	objectCount++;
}

VkDescriptorSetLayout Model::modelMatrixDSLayout = VK_NULL_HANDLE;

// private:

uint32_t Model::objectCount = 0;


