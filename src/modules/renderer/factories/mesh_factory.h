//
// Created by lepag on 6/18/2025.
//

#pragma once

#include "types.h"
#include "resources/mesh.h"

namespace GyroEngine::Factories
{
    class MeshFactory
    {
    public:
        static Resources::MeshHandle CreateCube();
    };

}
