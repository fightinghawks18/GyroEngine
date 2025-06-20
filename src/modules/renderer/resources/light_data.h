//
// Created by lepag on 6/20/2025.
//

#pragma once

#include <glm/glm.hpp>

enum class LightType : uint8_t
{
    Point,
    Directional,
    Spot
};

struct LightData
{
    glm::vec3 position;
    glm::vec3 direction;
    glm::vec3 color;
    float intensity;
    float radius;
    float spotAngle;
};