//
// Created by lepag on 6/10/2025.
//

#pragma once

#include "../render_step.h"

class ClearStep final : public IRenderStep {
public:
    ~ClearStep() override;
    void execute(Renderer& renderer) override;
};
