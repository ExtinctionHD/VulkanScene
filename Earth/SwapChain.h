#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include "VkDeleter.h"


class SwapChain
{
public:
	// properties
	VkExtent2D extent;
	uint32_t imageCount;
	VkFormat imageFormat;

	// images to rendering that swapchain provides
	std::vector<VkImage> images;

	operator VkSwapchainKHR();

	// reference to the device in order not to clone the VkDevice object
	void create(Device &device, VkSurfaceKHR surface, VkExtent2D surfaceExtent);

private:
	VkDeleter<VkSwapchainKHR> swapChain;

	// get real image count and images themself
	void getImages(VkDevice device);

	// try to found this surface format
	const VkSurfaceFormatKHR preferredFormat = { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };

	// try to found this present mode
	const VkPresentModeKHR preferredPresentMode = { VK_PRESENT_MODE_MAILBOX_KHR };

	// swapchain display frames on surface, so exactly swapchain sets format of surface
	VkSurfaceFormatKHR chooseSurfaceFormat(std::vector<VkSurfaceFormatKHR> availableFormats);

	// choose order of displaying framebuffers
	VkPresentModeKHR choosePresentMode(std::vector<VkPresentModeKHR> availablePresentModes);

	static VkExtent2D chooseExtent(VkSurfaceCapabilitiesKHR capabilities, VkExtent2D actualExtent);

	// minimal image count, real count can be greate
	static uint32_t chooseImageCount(VkSurfaceCapabilitiesKHR capabilities);
};

