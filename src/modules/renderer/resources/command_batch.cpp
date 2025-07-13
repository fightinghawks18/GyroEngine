//
// Created by lepag on 7/11/2025.
//

#include "command_batch.h"

#include "commands/clear_color_command.h"
#include "commands/clear_depth_command.h"
#include "commands/material_command.h"
#include "rendering/renderer.h"
#include "commands/mesh_command.h"
#include "commands/sky_command.h"

namespace GyroEngine::Resources
{
    CommandBatch::CommandBatch(Renderer &renderer)
        : m_renderer(renderer)
    {
    }

    CommandBatch::~CommandBatch()
    {
    }

    void CommandBatch::SubmitMesh(const uint32_t materialID, const BufferHandle &vertexBuffer,
                                  const BufferHandle &indexBuffer)
    {
        auto meshCommand = std::make_unique<MeshCommand>();
        meshCommand->vertexBuffer = vertexBuffer;
        meshCommand->indexBuffer = indexBuffer;
        meshCommand->material = materialID;
        m_commands.push_back(std::move(meshCommand));
    }

    void CommandBatch::SubmitMaterial(const uint32_t materialID, const MaterialHandle &material)
    {
        const auto it = std::find_if(m_commands.begin(), m_commands.end(), [&](const MaterialCommand &command)
        {
            return command.id == materialID || command.material == material;
        });

        if (it != m_commands.end())
        {
            return; // Materials should not be bound more than once per frame
        }

        auto materialCommand = std::make_unique<MaterialCommand>();
        materialCommand->material = material;
        materialCommand->id = materialID;
        m_commands.push_back(std::move(materialCommand));
    }

    void CommandBatch::SubmitColorClear(std::array<float, 4> color)
    {
        auto colorClearCommand = std::make_unique<ClearColorCommand>();
        colorClearCommand->color = color;
        m_commands.push_back(std::move(colorClearCommand));
    }

    void CommandBatch::SubmitDepthClear(const uint32_t depth = 0, const uint32_t stencil = 1.0)
    {
        auto depthClearCommand = std::make_unique<ClearDepthCommand>();
        depthClearCommand->depth = depth;
        depthClearCommand->stencil = stencil;
        m_commands.push_back(std::move(depthClearCommand));
    }

    void CommandBatch::Execute()
    {
        const auto frame = m_renderer.GetFrameContext();

        ExecuteForCommandType<ClearColorCommand>([&](auto command)
        {
            auto clearColorAttachment = Utils::Renderer::CreateRenderAttachment(frame.colorImage->GetImageView());
            clearColorAttachment.clearValue.color = command->color;

            const auto renderInfo = Utils::Renderer::CreateRenderingInfo(
                {{0, 0}, frame.swapchainExtent},
                {clearColorAttachment}
            );

            m_renderer.StartRender(renderInfo);
            m_renderer.EndRender();
        });

        ExecuteForCommandType<ClearDepthCommand>([&](auto command)
        {
            auto clearDepthAttachment = Utils::Renderer::CreateRenderAttachment(frame.colorImage->GetImageView());
            clearDepthAttachment.clearValue.depthStencil = {command->depth, command->stencil};

            const auto renderInfo = Utils::Renderer::CreateRenderingInfo(
                {{0, 0}, frame.swapchainExtent},
                {},
                clearDepthAttachment
            );

            m_renderer.StartRender(renderInfo);
            m_renderer.EndRender();
        });

        ExecuteForCommandType<SkyCommand>([&](auto command)
        {
            auto clearColorAttachment = Utils::Renderer::CreateRenderAttachment(
                frame.colorImage->GetImageView(),
                VK_ATTACHMENT_LOAD_OP_LOAD,
                VK_ATTACHMENT_STORE_OP_STORE);
            clearColorAttachment.clearValue.color = command->color;

            auto clearDepthAttachment = Utils::Renderer::CreateRenderAttachment(
                frame.colorImage->GetImageView(),
                VK_ATTACHMENT_LOAD_OP_LOAD,
                VK_ATTACHMENT_STORE_OP_STORE);
            clearDepthAttachment.clearValue.depthStencil = {command->depth, command->stencil};

            const auto renderInfo = Utils::Renderer::CreateRenderingInfo(
                {{0, 0}, frame.swapchainExtent},
                {clearColorAttachment},
                clearDepthAttachment
            );

            m_renderer.StartRender(renderInfo);
            command->skyPipeline->Bind(frame);
            command->skyPipeline->DrawFullscreenQuad(frame);
            m_renderer.EndRender();
        });

        std::unordered_map<uint32_t, MaterialCommand *> idToMaterialMap;
        std::unordered_map<uint32_t, std::vector<MeshCommand *> > idToMeshesMap;

        // Add materials to map for quick lookup
        ExecuteForCommandType<MaterialCommand>([&idToMaterialMap](auto command)
        {
            idToMaterialMap[command->id] = command;
        });

        // We cannot do mesh operations if there is no materials to use
        if (!idToMaterialMap.empty())
        {
            // Add meshes to a map so that we can bind and draw in batches
            ExecuteForCommandType<MaterialCommand>([&idToMaterialMap, &idToMeshesMap](auto command)
            {
                if (!idToMaterialMap[command->material]) return;
                idToMeshesMap[command->material].push_back(command);
            });

            // Start binding and drawing objects
            for (const auto &[id, meshCommands]: idToMeshesMap)
            {
                if (meshCommands.empty()) continue;

                const auto materialCommand = idToMaterialMap[id];
                materialCommand->pipeline->Bind(frame);

                for (const auto &command: meshCommands)
                {
                    command->indexBuffer->Bind(frame);
                    command->vertexBuffer->Bind(frame);
                    vkCmdDrawIndexed(frame.cmd, command->indexCount, 1, 0, 0, 0);
                }
            }
        }

        // Commands only exist to dictate what is going to happen for a single frame,
        // so we must clear them after execution
        m_commands.clear();
    }
}
