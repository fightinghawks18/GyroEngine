//
// Created by lepag on 7/11/2025.
//

#pragma once

#include <cstdint>

#include "command.h"

struct ClearDepthCommand : ICommand
{
    uint32_t depth = 1;
    uint32_t stencil = 0;
};