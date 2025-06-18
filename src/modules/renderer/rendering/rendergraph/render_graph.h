//
// Created by lepag on 6/17/2025.
//

#pragma once

#include <memory>
#include <vector>
#include "render_pass.h"


namespace GyroEngine::Rendering
{

    class RenderGraph
    {
        using Pass = std::shared_ptr<Passes::IRenderPass>;
    public:
        RenderGraph(): m_debugging(false)
        {
        }

        void SetDebug(const bool debug = false) { m_debugging = debug; }
        void AddPass(const Pass& pass);
        void RemovePass(const Pass& pass);
        void ClearPasses();
        void Execute(Renderer& renderer);
    private:
        std::vector<Pass> m_passes;
        bool m_debugging;

        [[nodiscard]] bool WasPassAdded(const Pass& pass) const;
    };

}
