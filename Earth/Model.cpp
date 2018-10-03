#include "Model.h"

// public:

Model::~Model()
{
	objectCount--;

	if (objectCount == 0 && mvpDSLayout != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorSetLayout(pDevice->device, mvpDSLayout, nullptr);
		mvpDSLayout = VK_NULL_HANDLE;
	}

	delete(pMvpBuffer);
}

glm::mat4 Model::getModelMatrix()
{
	return mvp.model;
}

void Model::setModelMatrix(glm::mat4 model)
{
	mvp.model = model;
	pMvpBuffer->updateData(&model, sizeof(model), offsetof(MvpMatrices, model));
}

void Model::setViewMatrix(glm::mat4 view)
{
	mvp.view = view;
	pMvpBuffer->updateData(&view, sizeof(view), offsetof(MvpMatrices, view));
}

void Model::setProjectionMatrix(glm::mat4 proj)
{
	mvp.proj = proj;
	pMvpBuffer->updateData(&proj, sizeof(proj), offsetof(MvpMatrices, proj));
}

void Model::setMvpMatrices(MvpMatrices mvp)
{
	this->mvp = mvp;
	pMvpBuffer->updateData(&mvp, sizeof(mvp), 0);
}

void Model::initDescriptorSets(DescriptorPool * pDescriptorPool)
{
	mvpDescriptorSet = pDescriptorPool->getDescriptorSet({ pMvpBuffer }, { }, Model::mvpDSLayout == VK_NULL_HANDLE, Model::mvpDSLayout);

	initMeshDescriptorSets(pDescriptorPool);
}

// protected:

Model::Model(Device *pDevice)
{
	this->pDevice = pDevice;

	pMvpBuffer = new Buffer(pDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_SHADER_STAGE_VERTEX_BIT, sizeof(MvpMatrices));
	setMvpMatrices({ glm::mat4(1.0f), glm::mat4(1.0f), glm::mat4(1.0f) });

	objectCount++;
}

// private:

uint32_t Model::objectCount = 0;

VkDescriptorSetLayout Model::mvpDSLayout = VK_NULL_HANDLE;

