#pragma once

#include "Device.h"
#include "SwapChainImage.h"

class Image : 
	public SwapChainImage
{
public:
    Image(
		Device *device,
		VkExtent3D extent,
		VkImageCreateFlags flags,
		VkSampleCountFlagBits sampleCount,
		uint32_t mipLevels,
		VkFormat format,
		VkImageTiling tiling,
		VkImageUsageFlags usage,
		uint32_t arrayLayers,
		bool cubeMap,
		VkMemoryPropertyFlags properties,
		VkImageAspectFlags aspectFlags);

	~Image();

	VkExtent3D getExtent() const;

	VkSampleCountFlagBits getSampleCount() const;

	void transitLayout(
        Device *device,
        VkImageLayout oldLayout,
        VkImageLayout newLayout,
        VkImageSubresourceRange subresourceRange) const;

	// load pixels in image memory 
	// pixel size depend from image format
	// memory size must be equals width * height * pixel size
	void updateData(std::vector<const void*> data, uint32_t layersOffset, uint32_t pixelSize) const;

	static void copyImage(
		Device *device, 
		Image *srcImage, 
		Image *dstImage,
		VkExtent3D extent,
		VkImageSubresourceLayers subresourceLayers);

protected:
	Image() = default;

	VkExtent3D extent;

	VkSampleCountFlagBits sampleCount;

	uint32_t mipLevels;

	uint32_t arrayLayers;

	void createThis(
		Device *device,
		VkExtent3D extent,
		VkImageCreateFlags flags,
		VkSampleCountFlagBits sampleCount,
		uint32_t mipLevels,
		VkFormat format,
		VkImageTiling tiling,
		VkImageUsageFlags usage,
		uint32_t arrayLayers,
		bool cubeMap,
		VkMemoryPropertyFlags properties,
		VkImageAspectFlags aspectFlags);

private:
	VkDeviceMemory memory;

	void allocateMemory(Device *device, VkMemoryPropertyFlags properties);
};

