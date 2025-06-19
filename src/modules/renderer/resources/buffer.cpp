//
// Created by lepag on 6/9/2025.
//

#include "buffer.h"

#include "context/rendering_device.h"
#include "rendering/renderer.h"

namespace GyroEngine::Resources
{
    Buffer& Buffer::SetBufferType(const BufferType& bufferType)
{
    m_bufferType = bufferType;
    return *this;
}

Buffer& Buffer::SetSize(VkDeviceSize size)
{
    m_size = size;
    return *this;
}

Buffer& Buffer::SetUsage(VkBufferUsageFlags usage)
{
    m_usage = usage;
    return *this;
}

Buffer& Buffer::SetMemoryUsage(const VmaMemoryUsage memoryUsage)
{
    m_memoryUsage = memoryUsage;
    return *this;
}

Buffer& Buffer::SetSharingMode(VkSharingMode sharingMode)
{
    m_sharingMode = sharingMode;
    return *this;
}

bool Buffer::Init()
{
    if (!CreateBuffer())
    {
        return false;
    }
    return true;
}

void Buffer::Cleanup()
{
    DestroyBuffer();
}

void Buffer::Bind(const Rendering::FrameContext& frameContext) const
{
    switch (m_bufferType)
    {
    case BufferType::Vertex:
        {
            constexpr VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(frameContext.cmd, 0, 1, &m_buffer, offsets);
        }
        break;
    case BufferType::Index:
        vkCmdBindIndexBuffer(frameContext.cmd, m_buffer, 0, VK_INDEX_TYPE_UINT32);
        break;
    default:
        break;
    }
}

void Buffer::Map(const void* data) const
{
    if (m_allocation != VK_NULL_HANDLE)
    {
        void* mappedData;
        if (vmaMapMemory(m_device.GetAllocator(), m_allocation, &mappedData) == VK_SUCCESS)
        {
            std::memcpy(mappedData, data, m_size);
            vmaUnmapMemory(m_device.GetAllocator(), m_allocation);
        }
        else
        {
            Logger::LogError("Failed to map buffer memory");
        }
    }
}

bool Buffer::CreateBuffer()
{
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = m_size;
    bufferInfo.usage = m_usage;
    bufferInfo.sharingMode = m_sharingMode;

    // Only set queue family indices if using concurrent sharing mode
    if (m_sharingMode == VK_SHARING_MODE_CONCURRENT)
    {
        const auto& families = m_device.GetDeviceFamilies();
        uint32_t indices[] = {families.GetGraphicsQueue().family, families.GetTransferQueue().family};
        uint32_t count = 0;
        for (const uint32_t index : indices)
        {
            if (index != VK_QUEUE_FAMILY_IGNORED)
            {
                count++;
            }
        }

        bufferInfo.queueFamilyIndexCount = count;
        bufferInfo.pQueueFamilyIndices = indices;
    }

    VmaAllocationCreateInfo vmaInfo = {};
    vmaInfo.usage = m_memoryUsage;
    vmaInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

    if (vmaCreateBuffer(m_device.GetAllocator(), &bufferInfo, &vmaInfo, &m_buffer, &m_allocation, nullptr) !=
        VK_SUCCESS)
    {
        Logger::LogError("Failed to create buffer");
        return false;
    }
    return true;
}

void Buffer::DestroyBuffer()
{
    if (m_buffer != VK_NULL_HANDLE && m_allocation != VK_NULL_HANDLE)
    {
        vmaDestroyBuffer(m_device.GetAllocator(), m_buffer, m_allocation);
        m_buffer = VK_NULL_HANDLE;
        m_allocation = VK_NULL_HANDLE;
    }
}
}
