//
// Created by lepag on 7/11/2025.
//

#pragma once

#include <memory>
#include <vector>
#include <array>
#include <functional>

#include "buffer/buffer.h"
#include "commands/command.h"
#include "object/material.h"

namespace GyroEngine::Rendering
{
    class Renderer;
}
using namespace GyroEngine::Rendering;

namespace GyroEngine::Resources
{

    class CommandBatch
    {
    public:
        explicit CommandBatch(Renderer& renderer);
        ~CommandBatch();

        void SubmitMesh(uint32_t materialID,
            const BufferHandle &vertexBuffer,
            const BufferHandle &indexBuffer);

        void SubmitMaterial(uint32_t materialID, const MaterialHandle& material);

        void SubmitColorClear(std::array<float, 4> color);
        void SubmitDepthClear(uint32_t depth, uint32_t stencil);

        void Execute();
    private:
        Renderer& m_renderer;
        std::vector<std::unique_ptr<ICommand>> m_commands;

        template <typename T>
        void ExecuteForCommandType(const std::function<void(T* command)>& callback)
        {
            for (auto& command : m_commands)
            {
                T* cmd = dynamic_cast<T*>(command.get());
                if (cmd == nullptr)
                    continue;

                callback(cmd);
            }
        }
    };

}
