//
// Created by lepag on 6/10/2025.
//

#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace types
{
    struct Vertex
    {
        glm::vec3 position;
        glm::vec3 color;

        Vertex(const glm::vec3& pos, const glm::vec3& col)
       : position(pos), color(col) {}
    };

    struct Transform
    {
        glm::vec3 position;
        glm::vec3 rotation;
        glm::vec3 scale;

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
}