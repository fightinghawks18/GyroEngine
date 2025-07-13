//
// Created by lepag on 7/11/2025.
//

#pragma once
#include "command.h"
#include "resources/buffer/buffer.h"

struct MeshCommand final : ICommand
{
    Resources::BufferHandle vertexBuffer;
    Resources::BufferHandle indexBuffer;
    uint32_t indexCount;
    uint32_t material;
};
