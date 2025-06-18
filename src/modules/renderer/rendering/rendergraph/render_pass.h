//
// Created by lepag on 6/17/2025.
//

#pragma once

#include "rendering/renderer.h"

namespace GyroEngine::Rendering::Passes
{

    class IRenderPass
    {
    public:
        IRenderPass(const std::string& name) : m_name(name)
        {
        }
        virtual ~IRenderPass() = default;
        virtual void Execute(Renderer& renderer) = 0;

        [[nodiscard]] const std::string& GetName() const
        {
            return m_name;
        }
    protected:
        std::string m_name;
    };

}
