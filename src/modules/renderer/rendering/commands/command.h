//
// Created by lepag on 6/19/2025.
//

#pragma once

namespace GyroEngine::Rendering
{
    class Renderer;
    struct FrameContext;
}

namespace GyroEngine::Rendering::Commands
{

    class ICommand
    {
        virtual ~ICommand() = default;
        virtual void Execute(Renderer& renderer, FrameContext& frame) = 0;
    };

}
