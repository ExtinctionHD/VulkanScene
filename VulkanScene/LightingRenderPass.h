#pragma once

#include "RenderPass.h"
#include "SwapChain.h"

class LightingRenderPass : public RenderPass
{
public:
	LightingRenderPass(Device *device, SwapChain *swapChain);

	std::shared_ptr<Image> getColorImage() const;

protected:
    void createAttachments() override;

    void createRenderPass() override;

    void createFramebuffers() override;

private:
	VkFormat colorAttachmentFormat;

	std::shared_ptr<Image> colorImage{};
};

