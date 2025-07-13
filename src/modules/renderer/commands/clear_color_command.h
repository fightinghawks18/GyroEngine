//
// Created by lepag on 7/11/2025.
//

#pragma once

#include <array>

#include "command.h"

struct ClearColorCommand : ICommand
{
    std::array<float, 4> color;
};