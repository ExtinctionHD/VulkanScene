#pragma once

#include "Device.h"
#include "SwapChainImage.h"

// allocates memory and creates new image,
// can create image view, and transit its layout
class Image : public SwapChainImage
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

	VkSampleCountFlagBits getSampleCount() const;

	void transitLayout(Device *device, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageSubresourceRange subresourceRange) const;

	void updateData(std::vector<const void*>, uint32_t layersOffset, uint32_t pixelSize) const;

	static void copyImage(
		Device *device,
		Image &srcImage,
		Image &dstImage,
		VkExtent3D extent,
		VkImageSubresourceLayers subresourceLayers
	);

protected:
	Image() = default;

	VkExtent3D extent;

	VkSampleCountFlagBits sampleCount;

	uint32_t mipLevels;

	uint32_t arrayLayers;

	void createThisImage(
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

