//
// Created by lepag on 6/10/2025.
//

#include "descriptor_manager.h"

#include "context/rendering_device.h"

DescriptorManager::DescriptorManager(RenderingDevice& device)
    : m_device(device)
{
}

DescriptorManager::~DescriptorManager() = default;

void DescriptorManager::cleanup()
{
    m_device.waitIdle();
    for (const auto& pool : m_descriptorPools) {
        pool->cleanup();
    }
    for (const auto& layout : m_descriptorLayouts) {
        layout->cleanup();
    }
    for (const auto& set : m_descriptorSets) {
        set->cleanup();
    }
    m_descriptorPools.clear();
    m_descriptorLayouts.clear();
    m_descriptorSets.clear();
}

std::shared_ptr<DescriptorPool> DescriptorManager::createDescriptorPool()
{
    auto pool = std::make_shared<DescriptorPool>(m_device);
    m_descriptorPools.push_back(pool);
    return pool;
}

std::shared_ptr<DescriptorLayout> DescriptorManager::createDescriptorLayout()
{
    auto layout = std::make_shared<DescriptorLayout>(m_device);
    m_descriptorLayouts.push_back(layout);
    return layout;
}

std::shared_ptr<DescriptorSet> DescriptorManager::createDescriptorSet(const std::shared_ptr<DescriptorPool>& pool,
    const std::shared_ptr<DescriptorLayout>& layout)
{
    auto set = std::make_shared<DescriptorSet>(m_device);
    set->setLayout(layout->getDescriptorSetLayout());
    set->setPool(pool->getDescriptorPool());
    m_descriptorSets.push_back(set);
    return set;
}
