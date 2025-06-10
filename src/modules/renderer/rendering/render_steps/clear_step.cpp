//
// Created by lepag on 6/10/2025.
//

#include "clear_step.h"
#include "../renderer.h"

ClearStep::~ClearStep()
{
}

void ClearStep::execute(Renderer& renderer)
{
    const FrameContext frame = renderer.getFrameContext();

    VkRenderingAttachmentInfoKHR colorAttachment{};
    colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
    colorAttachment.imageView = frame.swapchainImage->getImageView();
    colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.clearValue.color = { 1.0f, 0.0f, 1.0f, 1.0f };

    VkRenderingInfoKHR renderingInfo{};
    renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
    renderingInfo.renderArea = { { 0, 0 }, frame.swapchainExtent };
    renderingInfo.layerCount = 1;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachments = &colorAttachment;

    vkCmdBeginRenderingKHR(frame.cmd, &renderingInfo);
    vkCmdEndRenderingKHR(frame.cmd);
}
