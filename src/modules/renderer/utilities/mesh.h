//
// Created by lepag on 6/19/2025.
//

#pragma once

#include <stdexcept>
#include <string>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "../utilities/types.h"

namespace GyroEngine::Utils::Mesh
{
    struct MeshFile
    {
        std::vector<Types::Vertex> vertices;
        std::vector<uint32_t> indices;
    };

    static MeshFile LoadMeshDataFromFile(const std::string& filePath)
    {
        MeshFile meshFile;

        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(filePath, aiProcess_Triangulate | aiProcess_CalcTangentSpace);
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            throw std::runtime_error("Failed to load mesh file: " + std::string(importer.GetErrorString()));
        }

        for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
        {
            const aiMesh* mesh = scene->mMeshes[i];
            for (unsigned int j = 0; j < mesh->mNumVertices; ++j)
            {
                glm::vec3 position = {mesh->mVertices[j].x, mesh->mVertices[j].y, mesh->mVertices[j].z};
                glm::vec3 normal = {mesh->mNormals[j].x, mesh->mNormals[j].y, mesh->mNormals[j].z};
                glm::vec2 texCoords = (mesh->HasTextureCoords(0)) ? glm::vec2(mesh->mTextureCoords[0][j].x, mesh->mTextureCoords[0][j].y) : glm::vec2(0.0f, 0.0f);
                glm::vec3 tangent = (mesh->HasTangentsAndBitangents()) ? glm::vec3(mesh->mTangents[j].x, mesh->mTangents[j].y, mesh->mTangents[j].z) : glm::vec3(0.0f);

                Types::Vertex vertex(position, normal, texCoords, tangent, glm::vec4(1.0f));
                meshFile.vertices.push_back(vertex);
            }

            for (unsigned int j = 0; j < mesh->mNumFaces; ++j)
            {
                const aiFace& face = mesh->mFaces[j];
                for (unsigned int k = 0; k < face.mNumIndices; ++k)
                {
                    meshFile.indices.push_back(face.mIndices[k]);
                }
            }
        }

        return meshFile;
    }

}
