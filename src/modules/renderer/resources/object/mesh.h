//
// Created by lepag on 6/18/2025.
//

#pragma once

#include "../buffer/buffer.h"
#include "../pipeline/pipeline.h"
#include "types.h"

namespace GyroEngine::Device
{
    class RenderingDevice;
}

namespace GyroEngine::Resources
{

    class Mesh
    {
    public:
        explicit Mesh(Device::RenderingDevice& device): m_device(device) {}
        ~Mesh() { Destroy(); }

        Mesh& UseVertices(const std::vector<Types::Vertex>& vertices);
        Mesh& UseIndices(const std::vector<uint32_t>& indices);
        Mesh& UsePipeline(const std::shared_ptr<Pipeline>& pipeline);
        Mesh& UseObjectMap(const Types::ObjectMap& objectMap)
        {
            m_vertices = objectMap.vertices;
            m_indices = objectMap.indices;
            return *this;
        }

        bool Generate();
        void Destroy();

        void SetTransforms(const glm::mat4& view, const glm::mat4& proj);
        void Update(uint32_t frameIndex);
        void Bind(const Rendering::FrameContext& frame) const;
        void Draw(const Rendering::FrameContext& frame) const;

        [[nodiscard]] glm::vec3& GetPosition()
        {
            return m_transform.position;
        }
        [[nodiscard]] glm::vec3& GetRotation()
        {
            return m_transform.rotation;
        }
        [[nodiscard]] glm::vec3& GetScale()
        {
            return m_transform.scale;
        }

        [[nodiscard]] Pipeline* GetPipeline()
        {
            return m_pipeline;
        }
    private:
        Device::RenderingDevice& m_device;

        Pipeline* m_pipeline = nullptr;
        std::unique_ptr<Buffer> m_vertexBuffer;
        std::unique_ptr<Buffer> m_indexBuffer;
        BufferHandle m_mvpBuffer;

        uint32_t m_indexCount = 0;
        uint32_t m_vertexCount = 0;
        std::vector<Types::Vertex> m_vertices;
        std::vector<uint32_t> m_indices;
        Types::Transform m_transform;
        Types::MVP m_mvp;

        bool m_isBuilt = false;
        bool m_pipelineDirty = false;

        bool CreateBuffers();
        void DestroyBuffers();

        void FillBuffers() const;

        bool RegenerateObject();
    };

    using MeshHandle = std::shared_ptr<Mesh>;
}
