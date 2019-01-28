#include "ToneRenderPass.h"
#include <cassert>


ToneRenderPass::ToneRenderPass(Device *device, SwapChain *swapChain)
    : RenderPass(device, swapChain->getExtent(), VK_SAMPLE_COUNT_1_BIT)
{
	this->swapChain = swapChain;
}

uint32_t ToneRenderPass::getColorAttachmentCount() const
{
	return 1;
}

void ToneRenderPass::createAttachments()
{
}

void ToneRenderPass::createRenderPass()
{
	const VkAttachmentDescription colorAttachmentDesc{
		0,
		swapChain->getImageFormat(),
		VK_SAMPLE_COUNT_1_BIT,
		VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		VK_ATTACHMENT_STORE_OP_STORE,
		VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		VK_ATTACHMENT_STORE_OP_DONT_CARE,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
	};

	VkAttachmentReference colorAttachmentRef{
		0,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	};

	// subpass and it dependencies (contain references)

	VkSubpassDescription subpass{
		0,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		0,
		nullptr,
		1,
		&colorAttachmentRef,
		nullptr,
		nullptr,
		0,
		nullptr
	};

	const VkSubpassDependency inputDependency{
		VK_SUBPASS_EXTERNAL,
		0,
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_ACCESS_MEMORY_READ_BIT,
		VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		VK_DEPENDENCY_BY_REGION_BIT,
	};

	const VkSubpassDependency outputDependency{
		0,
		VK_SUBPASS_EXTERNAL,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		VK_ACCESS_MEMORY_READ_BIT,
		VK_DEPENDENCY_BY_REGION_BIT,
	};

	std::vector<VkSubpassDependency> dependencies{
		inputDependency,
		outputDependency
	};

	// render pass (contain descriptions)

	VkRenderPassCreateInfo createInfo{
		VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		nullptr,
		0,
		1,
		&colorAttachmentDesc,
		1,
		&subpass,
		uint32_t(dependencies.size()),
		dependencies.data(),
	};

	const VkResult result = vkCreateRenderPass(device->get(), &createInfo, nullptr, &renderPass);
	assert(result == VK_SUCCESS);
}

void ToneRenderPass::createFramebuffers()
{
	std::vector<VkImageView> swapChainImageViews = swapChain->getImageViews();

	for (auto swapChainImageView : swapChainImageViews)
	{
		addFramebuffer({ swapChainImageView });
	}
}
