//
// Created by lepag on 6/9/2025.
//

#include "buffer.h"

#include "context/rendering_device.h"

Buffer & Buffer::setSize(VkDeviceSize size)
{
    m_size = size;
    return *this;
}

Buffer & Buffer::setUsage(VkBufferUsageFlags usage)
{
    m_usage = usage;
    return *this;
}

Buffer & Buffer::setMemoryUsage(VmaMemoryUsage memoryUsage)
{
    m_memoryUsage = memoryUsage;
    return *this;
}

Buffer & Buffer::setSharingMode(VkSharingMode sharingMode)
{
    m_sharingMode = sharingMode;
    return *this;
}

bool Buffer::init()
{
    if (!createBuffer())
    {
        return false;
    }
    return true;
}

void Buffer::cleanup()
{
    destroyBuffer();
}

void Buffer::map(const void *data)
{
    if (m_allocation != VK_NULL_HANDLE)
    {
        void* mappedData;
        if (vmaMapMemory(m_device.getAllocator(), m_allocation, &mappedData) == VK_SUCCESS)
        {
            std::memcpy(mappedData, data, m_size);
            vmaUnmapMemory(m_device.getAllocator(), m_allocation);
        }
        else
        {
            Printer::error("Failed to map buffer memory");
        }
    }
}

bool Buffer::createBuffer()
{
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = m_size;
    bufferInfo.usage = m_usage;
    bufferInfo.sharingMode = m_sharingMode;

    // Only set queue family indices if using concurrent sharing mode
    if (m_sharingMode == VK_SHARING_MODE_CONCURRENT) {
        auto& families = m_device.getDeviceFamilies();
        uint32_t indices[] = { families.getGraphicsQueue().family, families.getTransferQueue().family };
        uint32_t count = 0;
        for (uint32_t index : indices) {
            if (index != VK_QUEUE_FAMILY_IGNORED) {
                count++;
            }
        }

        bufferInfo.queueFamilyIndexCount = count;
        bufferInfo.pQueueFamilyIndices = indices;
    }

    VmaAllocationCreateInfo vmaInfo = {};
    vmaInfo.usage = m_memoryUsage;
    vmaInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

    if (vmaCreateBuffer(m_device.getAllocator(), &bufferInfo, &vmaInfo, &m_buffer, &m_allocation, nullptr) != VK_SUCCESS)
    {
        Printer::error("Failed to create buffer");
        return false;
    }
    return true;
}

void Buffer::destroyBuffer()
{
    if (m_buffer != VK_NULL_HANDLE && m_allocation != VK_NULL_HANDLE) {
        vmaDestroyBuffer(m_device.getAllocator(), m_buffer, m_allocation);
        m_buffer = VK_NULL_HANDLE;
        m_allocation = VK_NULL_HANDLE;
    }
}
