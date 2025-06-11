//
// Created by lepag on 6/9/2025.
//

#pragma once

#include <volk.h>

#include "device_resource.h"
#include "implementation/vma_implementation.h"

struct FrameContext;
class RenderingDevice;

class Buffer : public IDeviceResource {
public:
    enum class BufferType
    {
        Vertex,
        Index,
        Uniform
    };

    explicit Buffer(RenderingDevice& device): IDeviceResource(device) {}
    ~Buffer() override = default;

    Buffer& setBufferType(const BufferType& bufferType);
    Buffer& setSize(VkDeviceSize size);
    Buffer& setUsage(VkBufferUsageFlags usage);
    Buffer& setMemoryUsage(VmaMemoryUsage memoryUsage);
    Buffer& setSharingMode(VkSharingMode sharingMode);

    bool init() override;
    void cleanup() override;

    void bind(const FrameContext& frameContext) const;

    void map(const void* data);

    [[nodiscard]] VkBuffer getBuffer() const {
        return m_buffer;
    }

    [[nodiscard]] VkDeviceSize getSize() const {
        return m_size;
    }
private:
    VkBuffer m_buffer = VK_NULL_HANDLE;
    VmaAllocation m_allocation = VK_NULL_HANDLE;
    VkDeviceSize m_size = 0;
    VkBufferUsageFlags m_usage = 0;
    VmaMemoryUsage m_memoryUsage = VMA_MEMORY_USAGE_AUTO;
    VkSharingMode m_sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    BufferType m_bufferType = BufferType::Uniform;

    bool createBuffer();
    void destroyBuffer();
};
