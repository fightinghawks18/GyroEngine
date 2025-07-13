//
// Created by lepag on 7/11/2025.
//

#pragma once

#include "command.h"

struct SkyCommand final : ICommand
{
    Resources::PipelineHandle skyPipeline;
};
