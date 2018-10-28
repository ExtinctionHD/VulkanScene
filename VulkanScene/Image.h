#pragma once

#include "Device.h"
#include "SwapChainImage.h"

// allocates memory and creates new image,
// can create image view, and transit its layout
class Image : 
	public SwapChainImage
{
public:
	Image() {}

	Image(
		Device *pDevice,
		VkExtent3D extent,
		VkImageCreateFlags flags,
		uint32_t mipLevels,
		VkFormat format,
		VkImageTiling tiling,
		VkImageUsageFlags usage,
		VkMemoryPropertyFlags properties,
		uint32_t arrayLayers
	);

	~Image();

	// extent of this image
	VkExtent3D extent{};

	VkImageView view = VK_NULL_HANDLE;

	void createImageView(VkImageSubresourceRange subresourceRange, VkImageViewType viewType);

	void transitLayout(Device *pDevice, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageSubresourceRange subresourceRange);

	// load pixels in image memory 
	// pixel size depend from image format
	// memory size must be equals width * height * pixel size
	void updateData(uint8_t * pixels, size_t pixelSize, uint32_t arrayLayer);

	static void copyImage(
		Device *pDevice, 
		Image& srcImage, 
		Image& dstImage,
		VkExtent3D extent,
		VkImageSubresourceLayers subresourceLayers
	);

protected:
	// constructor method to use in derived class
	void createThisImage(
		Device *pDevice,
		VkExtent3D extent,
		VkImageCreateFlags flags,
		uint32_t mipLevels,
		VkFormat format,
		VkImageTiling tiling,
		VkImageUsageFlags usage,
		VkMemoryPropertyFlags properties,
		uint32_t arrayLayers
	);

private:
	// memory that stores this image
	VkDeviceMemory stagingMemory = VK_NULL_HANDLE;

	void allocateMemory(Device *pDevice, VkMemoryPropertyFlags properties);
};

