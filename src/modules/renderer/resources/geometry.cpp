//
// Created by lepag on 6/18/2025.
//

#include "geometry.h"

#include "debug/logger.h"
#include "rendering/renderer.h"


namespace GyroEngine::Resources
{
    Geometry & Geometry::UseVertices(const std::vector<Types::Vertex> &vertices)
    {
        m_vertices = vertices;
        return *this;
    }

    Geometry & Geometry::UseIndices(const std::vector<uint32_t> &indices)
    {
        m_indices = indices;
        return *this;
    }

    Geometry & Geometry::UsePipeline(const std::shared_ptr<Pipeline> &pipeline)
    {
        m_pipeline = pipeline.get();
        m_pipelineDirty = true;
        return *this;
    }

    bool Geometry::Generate()
    {
        if (m_isBuilt)
        {
            return RegenerateObject();
        }
        if (!CreateBuffers()) return false;
        if (!SetupUniforms()) return false;
        FillBuffers();
        m_isBuilt = true;
        return true;
    }

    void Geometry::Destroy()
    {
        if (!m_isBuilt)
        {
            return;
        }
        DestroyBuffers();
        m_isBuilt = false;
    }

    void Geometry::SetTransforms(const glm::mat4 &view, const glm::mat4 &proj)
    {
        m_mvp.view = view;
        m_mvp.projection = proj;
    }

    void Geometry::Update()
    {
        m_mvp.model = m_transform.ToMatrix();
        m_mvpBuffer->Map(&m_mvp);
        if (m_mvpDescriptor)
        {
            m_mvpDescriptor->UpdateBuffer(0, m_mvpBuffer->GetBuffer(), 0, m_mvpBuffer->GetSize());
        }
    }

    void Geometry::Draw(const Rendering::FrameContext &frameContext) const
    {
        m_pipeline->Bind(frameContext);
        if (m_mvpDescriptor)
        {
            m_mvpDescriptor->Bind(frameContext, m_pipeline->GetPipelineLayout());
        }
        m_vertexBuffer->Bind(frameContext);
        m_indexBuffer->Bind(frameContext);
        vkCmdDrawIndexed(frameContext.cmd, static_cast<uint32_t>(m_indices.size()), 1, 0, 0, 0);
    }

    bool Geometry::CreateBuffers()
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
        m_mvpBuffer = std::make_unique<Buffer>(m_device);
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

    void Geometry::DestroyBuffers()
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

    void Geometry::FillBuffers() const
    {
        m_vertexBuffer->Map(m_vertices.data());
        m_indexBuffer->Map(m_indices.data());
    }

    bool Geometry::SetupUniforms()
    {
        const auto descriptorManager = m_pipeline->GetDescriptorManager();
        if (!descriptorManager)
        {
            Logger::LogError("Pipeline has no descriptor manager");
            return false;
        }
        const auto descriptorSets = descriptorManager->GetDescriptorSets();
        const auto descriptorLayouts = descriptorManager->GetDescriptorLayouts();
        const auto descriptorPools = descriptorManager->GetDescriptorPools();
        if (descriptorSets.empty())
        {
            // Create the descriptor set for object
            // ^ Assume first pool and layout are for objects
            m_mvpDescriptor = descriptorManager->CreateDescriptorSet(descriptorPools[0], descriptorLayouts[0]);
            if (!m_mvpDescriptor->Init())
            {
                Logger::LogError("Failed to create object descriptor set");
                return false;
            }
        } else
        {
            m_mvpDescriptor = descriptorSets[0]; // Assume first is for object
        }
        return true;
    }

    bool Geometry::RegenerateObject()
    {
        if (m_pipelineDirty)
        {
            // Get new uniforms from the new pipeline
            SetupUniforms();
            m_pipelineDirty = false;
        }

        FillBuffers();
        return true;
    }
}
