#pragma once

#include <vulkan/vulkan.h>
#include <vector>

class SwapChain
{
public:
	SwapChain(Device *pDevice, VkSurfaceKHR surface, VkExtent2D surfaceExtent);

	// destroy objects: swapchain, imageViews
	~SwapChain();

	VkSwapchainKHR getSwapchain() const;

	std::vector<VkImageView> getImageViews() const;

	VkExtent2D getExtent() const;

	uint32_t getImageCount() const;

	VkFormat getImageFormat() const;

	void recreate(VkExtent2D newExtent);

private:
	// try to found this pSurface format
	const VkSurfaceFormatKHR PRESENT_FORMAT = { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };

	// try to found this present mode
	const VkPresentModeKHR PREFERRED_PRESENT_MODE = VK_PRESENT_MODE_MAILBOX_KHR;

	// device that provide swapchain
	Device *pDevice;

	VkSurfaceKHR surface;

	VkSwapchainKHR swapChain;

	// properties
	VkExtent2D extent;
	VkFormat imageFormat;

	// images to rendering that swapchain provides
	std::vector<VkImage> images;

	// view of each swapchain image
	std::vector<VkImageView> imageViews;

	void create(VkExtent2D surfaceExtent);

	// swapchain display frames on pSurface, so exactly swapchain sets format of pSurface
	VkSurfaceFormatKHR chooseSurfaceFormat(std::vector<VkSurfaceFormatKHR> availableFormats) const;

	// choose order of displaying framebuffers
	VkPresentModeKHR choosePresentMode(std::vector<VkPresentModeKHR> availablePresentModes) const;

	static VkExtent2D chooseExtent(VkSurfaceCapabilitiesKHR capabilities, VkExtent2D actualExtent);

	// minimal image count, real count can be greate
	static uint32_t chooseImageCount(VkSurfaceCapabilitiesKHR capabilities);

	// get real image count and images themself
	void getImages(uint32_t imageCount);

	void createImageViews();

	void cleanup();
};

