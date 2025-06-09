//
// Created by lepag on 6/9/2025.
//

#pragma once

#include <volk.h>

#include "implementation/vma_implementation.h"

class RenderingDevice;

class Buffer {
public:
    explicit Buffer(RenderingDevice& device)
        : m_device(device) {}
    ~Buffer() = default;

    Buffer& setSize(VkDeviceSize size);
    Buffer& setUsage(VkBufferUsageFlags usage);
    Buffer& setMemoryUsage(VmaMemoryUsage memoryUsage);
    Buffer& setSharingMode(VkSharingMode sharingMode);

    bool init();
    void cleanup();

    void map(const void* data);

    [[nodiscard]] VkBuffer getBuffer() const {
        return m_buffer;
    }

    [[nodiscard]] VkDeviceSize getSize() const {
        return m_size;
    }
private:
    RenderingDevice& m_device;

    VkBuffer m_buffer = VK_NULL_HANDLE;
    VmaAllocation m_allocation = VK_NULL_HANDLE;
    VkDeviceSize m_size = 0;
    VkBufferUsageFlags m_usage = 0;
    VmaMemoryUsage m_memoryUsage = VMA_MEMORY_USAGE_AUTO;
    VkSharingMode m_sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    bool createBuffer();
    void destroyBuffer();
};
