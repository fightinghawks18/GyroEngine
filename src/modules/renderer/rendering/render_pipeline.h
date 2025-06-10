//
// Created by lepag on 6/10/2025.
//

#pragma once

#include <memory>
#include <vector>

#include "render_step.h"

class Renderer;

class RenderPipeline {
public:
    explicit RenderPipeline(Renderer& renderer): m_renderer(renderer) {}
    ~RenderPipeline() = default;
    RenderPipeline& addStep(const std::shared_ptr<IRenderStep>& step);

    void execute();
private:
    Renderer& m_renderer;
    std::vector<std::shared_ptr<IRenderStep>> m_renderSteps;
};
