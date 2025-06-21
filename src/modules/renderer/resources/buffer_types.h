//
// Created by lepag on 6/20/2025.
//

#pragma once

#include <glm/glm.hpp>

namespace GyroEngine::Resources
{
    struct CameraBuffer
    {
        glm::vec3 position;
        float padding1;
        glm::vec3 direction;
        float padding2;
        glm::vec3 up;
        float fov;
        float aspect;
        float nearPlane;
        float farPlane;
        float padding3;
    };

    struct LightBuffer
    {
        int type;
        glm::vec3 position;
        float padding1;
        glm::vec3 direction;
        float padding2;
        glm::vec3 color;
        float padding3;
        float intensity;
        float range;
        float angle;
    };
}
