//
// Created by lepag on 6/17/2025.
//

#include "render_graph.h"


namespace GyroEngine::Rendering
{
    void RenderGraph::AddPass(const Pass &pass)
    {
        if (WasPassAdded(pass))
        {
            return;
        }
        m_passes.push_back(pass);
    }

    void RenderGraph::RemovePass(const Pass &pass)
    {
        const auto it = std::remove_if(m_passes.begin(), m_passes.end(),
                                 [&pass](const Pass& p) { return p == pass; });
        if (it != m_passes.end())
        {
            m_passes.erase(it, m_passes.end());
        }
    }

    void RenderGraph::ClearPasses()
    {
        m_passes.clear();
    }

    void RenderGraph::Execute(Renderer& renderer)
    {
        for (const auto &pass : m_passes)
        {
            if (m_debugging)
            {
                Logger::Log("Executing pass: " + pass->GetName());
            }
            pass->Execute(renderer);
        }
        ClearPasses();
    }

    bool RenderGraph::WasPassAdded(const Pass &pass) const
    {
        return std::find(m_passes.begin(), m_passes.end(), pass) != m_passes.end();
    }
}
