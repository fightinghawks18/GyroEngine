//
// Created by lepag on 6/18/2025.
//

#pragma once

#include "types.h"
#include "../resources/object/mesh.h"

namespace GyroEngine::Factories
{
    class MeshFactory
    {
    public:
        static Resources::MeshHandle CreateCube();
        static Resources::MeshHandle CreateFromFile(const std::string& filePath);
    };

}
