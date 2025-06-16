//
// Created by lepag on 6/10/2025.
//

#include "descriptor_pool.h"

#include "context/rendering_device.h"

DescriptorPool& DescriptorPool::setPoolSize(const std::vector<PoolSize>& poolSizes)
{
    m_descriptorPoolSizes = poolSizes;
    return *this;
}

DescriptorPool& DescriptorPool::addPoolSize(const PoolSize poolSize)
{
    m_descriptorPoolSizes.push_back(poolSize);
    return *this;
}

DescriptorPool& DescriptorPool::clearPoolSizes()
{
    m_descriptorPoolSizes.clear();
    return *this;
}

DescriptorPool & DescriptorPool::setMaxSets(uint32_t maxSets)
{
    m_maxSets = maxSets;
    return *this;
}

bool DescriptorPool::init()
{
    if (m_descriptorPool != VK_NULL_HANDLE)
    {
        return false;
    }
    return createDescriptorPool();
}

void DescriptorPool::cleanup()
{
    if (m_descriptorPool != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorPool(m_device.getLogicalDevice(), m_descriptorPool, nullptr);
        m_descriptorPool = VK_NULL_HANDLE;
    }
}

bool DescriptorPool::createDescriptorPool()
{
    std::vector<VkDescriptorPoolSize> poolSizes;
    poolSizes.reserve(m_descriptorPoolSizes.size());
    for (auto [type, maxSets] : m_descriptorPoolSizes)
    {
        poolSizes.push_back({type, maxSets});
    }

    VkDescriptorPoolCreateInfo descriptorPoolInfo = {};
    descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    descriptorPoolInfo.pPoolSizes = poolSizes.data();
    descriptorPoolInfo.maxSets = m_maxSets;
    descriptorPoolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

    VkResult result = vkCreateDescriptorPool(m_device.getLogicalDevice(), &descriptorPoolInfo, nullptr, &m_descriptorPool);
    if (result != VK_SUCCESS)
    {
        Printer::error("Failed to create descriptor pool");
        return false;
    }
    return true;
}
