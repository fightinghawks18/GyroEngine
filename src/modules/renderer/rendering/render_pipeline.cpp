//
// Created by lepag on 6/10/2025.
//

#include "render_pipeline.h"
#include "renderer.h"

RenderPipeline& RenderPipeline::addStep(const std::shared_ptr<IRenderStep>& step)
{
    m_renderSteps.push_back(step);
    return *this;
}

void RenderPipeline::execute()
{
    for (const auto& step : m_renderSteps)
    {
        step->execute(m_renderer);
    }
    m_renderSteps.clear();
}
