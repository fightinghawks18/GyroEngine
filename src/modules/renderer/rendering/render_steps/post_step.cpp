//
// Created by lepag on 6/16/2025.
//

#include "post_step.h"

#include "rendering/renderer.h"

PostStep::~PostStep()
{
}

void PostStep::execute(Renderer &renderer)
{
    const FrameContext frame = renderer.getFrameContext();

    uint32_t pingpong = 0;
    Image* inputImage = frame.colorImage;
    for (uint32_t i = 0; i < m_pipelines.size(); i++)
    {
        bool isFinalPost = (i == m_pipelines.size() - 1);
        Image* outputImage = isFinalPost
            ? frame.swapchainImage
            : frame.pipelineImages[pingpong];

        Pipeline* pipeline = m_pipelines[i].pipeline;
        DescriptorSet* descriptorSet = m_pipelines[i].descriptorSet;
        uint32_t binding = m_pipelines[i].binding;

        inputImage->makeShaderReadable();

        VkRenderingAttachmentInfoKHR colorAttachment{};
        colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
        colorAttachment.imageView = outputImage->getImageView();
        colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.clearValue.color = { 1.0f, 0.0f, 1.0f, 1.0f };

        VkExtent2D extent;
        if (isFinalPost)
        {
            extent = frame.swapchainExtent;
        } else
        {
            extent = { outputImage->getExtent().width, outputImage->getExtent().height };
        }

        VkRenderingInfoKHR renderingInfo{};
        renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
        renderingInfo.renderArea = { { 0, 0 }, extent };
        renderingInfo.layerCount = 1;
        renderingInfo.colorAttachmentCount = 1;
        renderingInfo.pColorAttachments = &colorAttachment;

        vkCmdBeginRenderingKHR(frame.cmd, &renderingInfo);

        pipeline->bind(frame);
        descriptorSet->bind(frame, pipeline->getPipelineLayout());
        descriptorSet->updateImage(binding, inputImage->getImageView(), frame.sampler->getSampler());
        pipeline->drawQuad(frame);

        vkCmdEndRenderingKHR(frame.cmd);

        if (isFinalPost)
        {
            outputImage->makePresent();

        } else
        {
            outputImage->makeShaderReadable();
        }

        inputImage = outputImage;
        pingpong = 1 - pingpong;
    }
}
