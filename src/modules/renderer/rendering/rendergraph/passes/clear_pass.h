//
// Created by lepag on 6/17/2025.
//

#pragma once

#include <array>
#include <glm/vec4.hpp>

#include "../render_pass.h"
#include "utilities/renderer.h"

namespace GyroEngine::Rendering::Passes
{

    class ClearPass final : public IRenderPass
    {
    public:
        ClearPass(): IRenderPass("ClearPass") {}
        ~ClearPass() override = default;

        void SetClearColor(const std::array<float, 4>& clearColor)
        {
            m_clearColor = clearColor;
        }

        void Execute(Renderer& renderer) override
        {
            auto frame = renderer.GetFrameContext();

            // Setup color attachment to clear colors on the screen
            auto colorAttachment = Utils::Renderer::CreateRenderAttachment(
                frame.swapchainImage->GetImageView());
            colorAttachment.clearValue.color = {m_clearColor[0], m_clearColor[1], m_clearColor[2], m_clearColor[3]};

            // Setup depth and stencil attachments to clear depth and stencil values
            auto depthAttachment = Utils::Renderer::CreateRenderAttachment(
                frame.depthImage->GetImageView(),
                VK_ATTACHMENT_LOAD_OP_CLEAR,
                VK_ATTACHMENT_STORE_OP_STORE,
                VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);
            depthAttachment.clearValue.depthStencil = {1.0f, 0};
            auto stencilAttachment = Utils::Renderer::CreateRenderAttachment(
                frame.depthImage->GetImageView(),
                VK_ATTACHMENT_LOAD_OP_CLEAR,
                VK_ATTACHMENT_STORE_OP_STORE,
                VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL);
            stencilAttachment.clearValue.depthStencil = {0, 1};

            std::vector colorAttachments = {colorAttachment};

            // Setup rendering info with attachments
            auto renderingInfo = Utils::Renderer::CreateRenderingInfo(
                {{0, 0}, frame.swapchainExtent},
                colorAttachments,
                depthAttachment,
                stencilAttachment);

            // Clear the attachments
            renderer.BindRenderingInfo(renderingInfo);
            renderer.StartRender();
            renderer.EndRender();
        }
    private:
        std::array<float, 4> m_clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
    };

}
