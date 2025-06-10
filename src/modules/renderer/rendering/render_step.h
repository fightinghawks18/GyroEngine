//
// Created by lepag on 6/10/2025.
//

#pragma once

class Renderer;

class IRenderStep
{
    public:
    IRenderStep() = default;
    virtual ~IRenderStep() = default;

    virtual void execute(Renderer& renderer) = 0;
};
