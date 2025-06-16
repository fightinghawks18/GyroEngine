//
// Created by lepag on 6/16/2025.
//

#pragma once

#include <vector>

#include "../render_step.h"
#include "resources/pipeline.h"

struct PostProcess
{
    Pipeline* pipeline;
    DescriptorSet* descriptorSet;
    uint32_t binding;
};

class PostStep : public IRenderStep {
public:
    ~PostStep() override;

    void addPipeline(PostProcess postprocess) {
        m_pipelines.push_back(postprocess);
    }

    void execute(Renderer &renderer) override;

private:
    std::vector<PostProcess> m_pipelines;
};
