//
// Created by lepag on 6/18/2025.
//

#include "mesh_factory.h"
#include <vector>

#include "utilities/mesh.h"
#include "../../../core/engine.h"

namespace GyroEngine::Factories
{
    Resources::MeshHandle MeshFactory::CreateCube()
    {
        auto device = Engine::Get().GetDeviceSmart();
        auto mesh = std::make_shared<Resources::Mesh>(*device);

        std::vector<Types::Vertex> vertices = {
        // Front face (+Z)
        {{-0.5f, -0.5f,  0.5f}, {0, 0, 1}, {0, 0}, {1, 0, 0}, {1, 0, 0, 1}},
        {{ 0.5f, -0.5f,  0.5f}, {0, 0, 1}, {1, 0}, {1, 0, 0}, {1, 0, 0, 1}},
        {{ 0.5f,  0.5f,  0.5f}, {0, 0, 1}, {1, 1}, {1, 0, 0}, {1, 0, 0, 1}},
        {{-0.5f,  0.5f,  0.5f}, {0, 0, 1}, {0, 1}, {1, 0, 0}, {1, 0, 0, 1}},
        // Back face (-Z)
        {{ 0.5f, -0.5f, -0.5f}, {0, 0, -1}, {0, 0}, {-1, 0, 0}, {0, 1, 0, 1}},
        {{-0.5f, -0.5f, -0.5f}, {0, 0, -1}, {1, 0}, {-1, 0, 0}, {0, 1, 0, 1}},
        {{-0.5f,  0.5f, -0.5f}, {0, 0, -1}, {1, 1}, {-1, 0, 0}, {0, 1, 0, 1}},
        {{ 0.5f,  0.5f, -0.5f}, {0, 0, -1}, {0, 1}, {-1, 0, 0}, {0, 1, 0, 1}},
        // Left face (-X)
        {{-0.5f, -0.5f, -0.5f}, {-1, 0, 0}, {0, 0}, {0, 0, -1}, {0, 0, 1, 1}},
        {{-0.5f, -0.5f,  0.5f}, {-1, 0, 0}, {1, 0}, {0, 0, -1}, {0, 0, 1, 1}},
        {{-0.5f,  0.5f,  0.5f}, {-1, 0, 0}, {1, 1}, {0, 0, -1}, {0, 0, 1, 1}},
        {{-0.5f,  0.5f, -0.5f}, {-1, 0, 0}, {0, 1}, {0, 0, -1}, {0, 0, 1, 1}},
        // Right face (+X)
        {{ 0.5f, -0.5f,  0.5f}, {1, 0, 0}, {0, 0}, {0, 0, 1}, {1, 1, 0, 1}},
        {{ 0.5f, -0.5f, -0.5f}, {1, 0, 0}, {1, 0}, {0, 0, 1}, {1, 1, 0, 1}},
        {{ 0.5f,  0.5f, -0.5f}, {1, 0, 0}, {1, 1}, {0, 0, 1}, {1, 1, 0, 1}},
        {{ 0.5f,  0.5f,  0.5f}, {1, 0, 0}, {0, 1}, {0, 0, 1}, {1, 1, 0, 1}},
        // Top face (+Y)
        {{-0.5f,  0.5f,  0.5f}, {0, 1, 0}, {0, 0}, {1, 0, 0}, {0, 1, 1, 1}},
        {{ 0.5f,  0.5f,  0.5f}, {0, 1, 0}, {1, 0}, {1, 0, 0}, {0, 1, 1, 1}},
        {{ 0.5f,  0.5f, -0.5f}, {0, 1, 0}, {1, 1}, {1, 0, 0}, {0, 1, 1, 1}},
        {{-0.5f,  0.5f, -0.5f}, {0, 1, 0}, {0, 1}, {1, 0, 0}, {0, 1, 1, 1}},
        // Bottom face (-Y)
        {{-0.5f, -0.5f, -0.5f}, {0, -1, 0}, {0, 0}, {1, 0, 0}, {1, 0, 1, 1}},
        {{ 0.5f, -0.5f, -0.5f}, {0, -1, 0}, {1, 0}, {1, 0, 0}, {1, 0, 1, 1}},
        {{ 0.5f, -0.5f,  0.5f}, {0, -1, 0}, {1, 1}, {1, 0, 0}, {1, 0, 1, 1}},
        {{-0.5f, -0.5f,  0.5f}, {0, -1, 0}, {0, 1}, {1, 0, 0}, {1, 0, 1, 1}},
    };

    std::vector<uint32_t> indices = {
        0, 1, 2, 2, 3, 0,       // Front
        4, 5, 6, 6, 7, 4,       // Back
        8, 9,10,10,11, 8,       // Left
       12,13,14,14,15,12,       // Right
       16,17,18,18,19,16,       // Top
       20,21,22,22,23,20        // Bottom
    };


        mesh->UseVertices(vertices);
        mesh->UseIndices(indices);
        return mesh;
    }

    Resources::MeshHandle MeshFactory::CreateFromFile(const std::string &filePath)
    {
        auto device = Engine::Get().GetDeviceSmart();
        auto mesh = std::make_shared<Resources::Mesh>(*device);
        auto [vertices, indices] = Utils::Mesh::LoadMeshDataFromFile(filePath);
        if (vertices.empty() || indices.empty())
        {
            Logger::LogError("Failed to load mesh from file: " + filePath);
            return nullptr;
        }

        mesh->UseVertices(vertices);
        mesh->UseIndices(indices);
        return mesh;
    }
}
