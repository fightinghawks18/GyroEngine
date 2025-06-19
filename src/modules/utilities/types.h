//
// Created by lepag on 6/10/2025.
//

#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace GyroEngine::Types
{
    struct Vertex
    {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 texCoords;
        glm::vec3 tangent;
        glm::vec4 color;

        Vertex(const glm::vec3& pos, const glm::vec3& nor, const glm::vec2& uv, const glm::vec3& tan, const glm::vec4& col)
       : position(pos), normal(nor), texCoords(uv), tangent(tan), color(col) {}
    };

    struct ObjectMap
    {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
    };

    struct Transform
    {
        glm::vec3 position = glm::vec3(0.0f);
        glm::vec3 rotation = glm::vec3(0.0f); // Euler angles in radians
        glm::vec3 scale = glm::vec3(1.0f);

        [[nodiscard]] glm::mat4 ToMatrix() const
        {
            const glm::mat4 translation = glm::translate(glm::mat4(1.0f), position);
            const glm::mat4 rotationX = glm::rotate(glm::mat4(1.0f), rotation.x, glm::vec3(1, 0, 0));
            const glm::mat4 rotationY = glm::rotate(glm::mat4(1.0f), rotation.y, glm::vec3(0, 1, 0));
            const glm::mat4 rotationZ = glm::rotate(glm::mat4(1.0f), rotation.z, glm::vec3(0, 0, 1));
            const glm::mat4 scaling = glm::scale(glm::mat4(1.0f), scale);

            return translation * rotationX * rotationY * rotationZ * scaling;
        }
    };

    struct MVP {
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = glm::mat4(1.0f);
        glm::mat4 projection = glm::mat4(1.0f);
    };
}