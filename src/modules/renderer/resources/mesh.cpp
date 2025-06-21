//
// Created by lepag on 6/18/2025.
//

#include "mesh.h"

#include "debug/logger.h"
#include "rendering/renderer.h"


namespace GyroEngine::Resources
{
    Mesh & Mesh::UseVertices(const std::vector<Types::Vertex> &vertices)
    {
        m_vertices = vertices;
        return *this;
    }

    Mesh & Mesh::UseIndices(const std::vector<uint32_t> &indices)
    {
        m_indices = indices;
        return *this;
    }

    Mesh & Mesh::UsePipeline(const std::shared_ptr<Pipeline> &pipeline)
    {
        m_pipeline = pipeline.get();
        m_pipelineDirty = true;
        return *this;
    }

    bool Mesh::Generate()
    {
        if (m_isBuilt)
        {
            return RegenerateObject();
        }
        if (!CreateBuffers()) return false;
        FillBuffers();
        m_isBuilt = true;
        return true;
    }

    void Mesh::Destroy()
    {
        if (!m_isBuilt)
        {
            return;
        }
        DestroyBuffers();
        m_isBuilt = false;
    }

    void Mesh::SetTransforms(const glm::mat4 &view, const glm::mat4 &proj)
    {
        m_mvp.view = view;
        m_mvp.projection = proj;
    }

    void Mesh::Update(const uint32_t frameIndex)
    {
        m_mvp.model = m_transform.ToMatrix();
        m_mvpBuffer->Map(&m_mvp);

        auto pipelineBindings = m_pipeline->GetPipelineBindings();
        if (pipelineBindings->DoesBindingExist("mvp"))
        {
            pipelineBindings->UpdateBufferSet("mvp", m_mvpBuffer, frameIndex);
        }
    }

    void Mesh::Bind(const Rendering::FrameContext& frame) const
    {
        m_pipeline->Bind(frame);
        m_indexBuffer->Bind(frame);
        m_vertexBuffer->Bind(frame);
        m_mvpBuffer->Bind(frame);

        auto pipelineBindings = m_pipeline->GetPipelineBindings();
        if (pipelineBindings->DoesBindingExist("mvp"))
        {
            pipelineBindings->BindSet("mvp", frame.cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline->GetPipelineLayout(), frame.frameIndex);
        }

        if (pipelineBindings->DoesBindingExist("usTexture"))
        {
            pipelineBindings->BindSet("usTexture", frame.cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline->GetPipelineLayout(), frame.frameIndex);
        }
    }

    void Mesh::Draw(const Rendering::FrameContext &frame) const
    {
        vkCmdDrawIndexed(frame.cmd, static_cast<uint32_t>(m_indices.size()), 1, 0, 0, 0);
    }

    bool Mesh::CreateBuffers()
    {
        // Create vertex buffer
        m_vertexBuffer = std::make_unique<Buffer>(m_device);
        m_vertexBuffer->SetUsage(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT)
            .SetMemoryUsage(VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE)
            .SetBufferType(Buffer::BufferType::Vertex)
            .SetSize(sizeof(Types::Vertex) * m_vertices.size());
        if (!m_vertexBuffer->Init())
        {
            Logger::LogError("Failed to create vertex buffer");
            return false;
        }

        // Create index buffer
        m_indexBuffer = std::make_unique<Buffer>(m_device);
        m_indexBuffer->SetUsage(VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT)
            .SetMemoryUsage(VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE)
            .SetBufferType(Buffer::BufferType::Index)
            .SetSize(sizeof(uint32_t) * m_indices.size());
        if (!m_indexBuffer->Init())
        {
            Logger::LogError("Failed to create index buffer");
            return false;
        }

        // Create MVP buffer (Model-View-Projection)
        m_mvpBuffer = std::make_shared<Buffer>(m_device);
        m_mvpBuffer->SetUsage(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
            .SetMemoryUsage(VMA_MEMORY_USAGE_AUTO_PREFER_HOST)
            .SetBufferType(Buffer::BufferType::Uniform)
            .SetSize(sizeof(Types::MVP));
        if (!m_mvpBuffer->Init())
        {
            Logger::LogError("Failed to create MVP buffer");
            return false;
        }
        return true;
    }

    void Mesh::DestroyBuffers()
    {
        if (m_vertexBuffer)
        {
            m_vertexBuffer->Cleanup();
            m_vertexBuffer.reset();
        }

        if (m_indexBuffer)
        {
            m_indexBuffer->Cleanup();
            m_indexBuffer.reset();
        }

        if (m_mvpBuffer)
        {
            m_mvpBuffer->Cleanup();
            m_mvpBuffer.reset();
        }
    }

    void Mesh::FillBuffers() const
    {
        m_vertexBuffer->Map(m_vertices.data());
        m_indexBuffer->Map(m_indices.data());
    }

    bool Mesh::RegenerateObject()
    {
        FillBuffers();
        return true;
    }
}
