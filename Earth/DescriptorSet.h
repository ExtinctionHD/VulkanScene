#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include "Buffer.h"
#include "TextureImage.h"
#include "Model.h"
#include "Vertex.h"

// provides a pipeline with resources: uniform buffers and textures
// deletes all resources and models during destroying
class DescriptorSet
{
public:
	DescriptorSet(Device *pDevice);
	~DescriptorSet();

	// layout of current resouces
	VkDescriptorSetLayout layout = VK_NULL_HANDLE;

	// updates layout, pool and set
	// call after resource change
	void update();

	void addBuffer(Buffer *pBuffer);

	void addTexture(TextureImage *pTexture);

	void removeBuffer(Buffer *pBuffer);

	void removeTexture(TextureImage *pTexture);

	void addModel(Model *pModel);

	void removeModel(Model *pModel);

	void bind(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout) const;

	void drawModels(VkCommandBuffer commandBuffer) const;

private:
	// device that provide descriptor set
	Device *pDevice;

	VkDescriptorPool pool = VK_NULL_HANDLE;

	VkDescriptorSet set = VK_NULL_HANDLE;

	// resources
	std::vector<Buffer*> uniformBuffers;
	std::vector<TextureImage*> textures;

	std::vector<Model*> models;

	// create layout of current resources
	void createLayout();

	// create pool of descriptors for current resources
	void createDescriptorPool();

	// create descriptor set of current resources
	void createDescriptorSet();

	// equals number of uniform buffers
	uint32_t getTextureBindingsOffset();
};

