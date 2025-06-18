//
// Created by lepag on 6/9/2025.
//

#pragma once

#include <volk.h>
#include "implementation/vma_implementation.h"

namespace GyroEngine::Device
{
    class RenderingDevice;
}

namespace GyroEngine::Rendering
{
    struct FrameContext;
}

using namespace GyroEngine;


namespace GyroEngine::Resources
{
    class Buffer  {
    public:
        enum class BufferType
        {
            Vertex,
            Index,
            Uniform
        };

        explicit Buffer(Device::RenderingDevice& device): m_device(device) {}
        ~Buffer() { Cleanup(); }

        Buffer& SetBufferType(const BufferType& bufferType);
        Buffer& SetSize(VkDeviceSize size);
        Buffer& SetUsage(VkBufferUsageFlags usage);
        Buffer& SetMemoryUsage(VmaMemoryUsage memoryUsage);
        Buffer& SetSharingMode(VkSharingMode sharingMode);

        bool Init();
        void Cleanup();

        void Bind(const Rendering::FrameContext& frameContext) const;

        void Map(const void* data) const;

        [[nodiscard]] VkBuffer GetBuffer() const {
            return m_buffer;
        }

        [[nodiscard]] VkDeviceSize GetSize() const {
            return m_size;
        }
    private:
        Device::RenderingDevice& m_device;

        VkBuffer m_buffer = VK_NULL_HANDLE;
        VmaAllocation m_allocation = VK_NULL_HANDLE;
        VkDeviceSize m_size = 0;
        VkBufferUsageFlags m_usage = 0;
        VmaMemoryUsage m_memoryUsage = VMA_MEMORY_USAGE_AUTO;
        VkSharingMode m_sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        BufferType m_bufferType = BufferType::Uniform;

        bool CreateBuffer();
        void DestroyBuffer();
    };
}
