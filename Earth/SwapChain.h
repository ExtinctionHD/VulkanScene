#pragma once

#include <vulkan/vulkan.h>
#include <vector>

class SwapChain
{
public:
	VkSwapchainKHR swapChain;

	// properties
	VkExtent2D extent;
	uint32_t imageCount;
	VkFormat imageFormat;

	// images to rendering that swapchain provides
	std::vector<VkImage> images;

	// view of each swapchain image
	std::vector<VkImageView> imageViews;

	SwapChain(Device *pDevice, VkSurfaceKHR surface, VkExtent2D surfaceExtent);

	// destroy objects: swapchain, imageViews
	~SwapChain();

	float getAspect() const;

private:
	// try to found this surface format
	const VkSurfaceFormatKHR PRESENT_FORMAT = { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };

	// try to found this present mode
	const VkPresentModeKHR PREFERRED_PRESENT_MODE = VK_PRESENT_MODE_MAILBOX_KHR;

	// device that provide swapchain
	VkDevice device;

	// swapchain display frames on surface, so exactly swapchain sets format of surface
	VkSurfaceFormatKHR chooseSurfaceFormat(std::vector<VkSurfaceFormatKHR> availableFormats) const;

	// choose order of displaying framebuffers
	VkPresentModeKHR choosePresentMode(std::vector<VkPresentModeKHR> availablePresentModes) const;

	static VkExtent2D chooseExtent(VkSurfaceCapabilitiesKHR capabilities, VkExtent2D actualExtent);

	// minimal image count, real count can be greate
	static uint32_t chooseImageCount(VkSurfaceCapabilitiesKHR capabilities);

	// get real image count and images themself
	void initImages();

	void createImageViews();
};

