//
// Created by lepag on 7/11/2025.
//

#pragma once

#include "command.h"
#include "resources/object/material.h"

struct MaterialCommand final : ICommand
{
    uint32_t id;
    Resources::PipelineHandle pipeline;
};
