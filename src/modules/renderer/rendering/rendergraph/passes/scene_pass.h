//
// Created by lepag on 6/18/2025.
//

#pragma once

#include <memory>

#include "../render_pass.h"
#include "resources/geometry.h"

namespace GyroEngine::Rendering::Passes
{

    class ScenePass final : public IRenderPass
    {
    public:
        ScenePass() : IRenderPass("Scene Pass") {}
        ~ScenePass() override = default;

        void AddGeometry(const std::shared_ptr<Resources::Geometry>& geometry)
        {
            m_geometries.push_back(geometry);
        }

        void Execute(Renderer& renderer) override
        {
            const auto frame = renderer.GetFrameContext();

            // Setup color attachment to clear colors on the screen
            auto colorAttachment = Utils::Renderer::CreateRenderAttachment(
                frame.swapchainImage->GetImageView(), VK_ATTACHMENT_LOAD_OP_LOAD);
            colorAttachment.clearValue.color = {0.0f, 0.0f, 0.0f, 1.f};

            // Setup depth and stencil attachments to clear depth and stencil values
            auto depthStencilAttachment = Utils::Renderer::CreateRenderAttachment(
                frame.depthImage->GetImageView(),
                VK_ATTACHMENT_LOAD_OP_LOAD,
                VK_ATTACHMENT_STORE_OP_STORE,
                VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);
            depthStencilAttachment.clearValue.depthStencil = {1.0f, 0};

            std::vector colorAttachments = {colorAttachment};

            // Setup rendering info with attachments
            auto renderingInfo = Utils::Renderer::CreateRenderingInfo(
                {{0, 0}, frame.swapchainExtent},
                colorAttachments,
                depthStencilAttachment);

            // Clear the attachments
            renderer.BindRenderingInfo(renderingInfo);
            renderer.StartRender();

            // Draw geometry
            for (const auto& geometry : m_geometries)
            {
                geometry->Update();
                geometry->Draw(frame);
            }
            m_geometries.clear();

            renderer.EndRender();
        }
    private:
        std::vector<std::shared_ptr<Resources::Geometry>> m_geometries;
    };

}
