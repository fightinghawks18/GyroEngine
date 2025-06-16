//
// Created by lepag on 6/10/2025.
//

#include "scene_step.h"

#include "rendering/renderer.h"

SceneStep::~SceneStep() = default;

void SceneStep::setPipeline(Pipeline* pipeline)
{
    m_pipeline = pipeline;
}

void SceneStep::setVertexBuffer(Buffer* vertexBuffer)
{
    m_vertexBuffer = vertexBuffer;
}

void SceneStep::setIndexBuffer(Buffer* indexBuffer)
{
    m_indexBuffer = indexBuffer;
}

void SceneStep::setVertices(const std::vector<types::Vertex>& vertices)
{
    m_vertices = vertices;
}

void SceneStep::setIndices(const std::vector<uint32_t>& indices)
{
    m_indices = indices;
}

void SceneStep::execute(Renderer& renderer)
{
    const FrameContext frame = renderer.getFrameContext();

    VkRenderingAttachmentInfoKHR colorAttachment{};
    colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
    colorAttachment.imageView = frame.colorImage->getImageView();
    colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.clearValue.color = { 1.0f, 0.0f, 0.0f, 1.0f };

    VkRenderingInfoKHR renderingInfo{};
    renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
    renderingInfo.renderArea = { { 0, 0 }, frame.swapchainExtent };
    renderingInfo.layerCount = 1;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachments = &colorAttachment;

    vkCmdBeginRenderingKHR(frame.cmd, &renderingInfo);

    m_pipeline->bind(frame);
    m_vertexBuffer->bind(frame);
    m_indexBuffer->bind(frame);

    vkCmdDrawIndexed(frame.cmd, m_indices.size(), 1, 0, 0, 0);

    vkCmdEndRenderingKHR(frame.cmd);
}
