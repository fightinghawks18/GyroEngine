//
// Created by lepag on 6/10/2025.
//

#include "descriptor_manager.h"

#include "context/rendering_device.h"

namespace GyroEngine::Resources
{
    DescriptorManager::DescriptorManager(Device::RenderingDevice& device)
    : m_device(device)
    {
    }

    DescriptorManager::~DescriptorManager() { Cleanup(); }

    void DescriptorManager::Cleanup()
    {
        m_device.WaitForIdle();
        for (const auto& pool : m_descriptorPools) {
            pool->Cleanup();
        }
        for (const auto& layout : m_descriptorLayouts) {
            layout->Cleanup();
        }
        for (const auto& set : m_descriptorSets) {
            set->SetPool(VK_NULL_HANDLE); // This set was freed already with the pool, avoid double free
            set->Cleanup();
        }
        m_descriptorPools.clear();
        m_descriptorLayouts.clear();
        m_descriptorSets.clear();
    }

    std::shared_ptr<DescriptorPool> DescriptorManager::CreateDescriptorPool()
    {
        auto pool = std::make_shared<DescriptorPool>(m_device);
        m_descriptorPools.push_back(pool);
        return pool;
    }

    std::shared_ptr<DescriptorLayout> DescriptorManager::CreateDescriptorLayout()
    {
        auto layout = std::make_shared<DescriptorLayout>(m_device);
        m_descriptorLayouts.push_back(layout);
        return layout;
    }

    std::shared_ptr<DescriptorSet> DescriptorManager::CreateDescriptorSet(const std::shared_ptr<DescriptorPool>& pool,
        const std::shared_ptr<DescriptorLayout>& layout)
    {
        auto set = std::make_shared<DescriptorSet>(m_device);
        set->SetLayout(layout->GetDescriptorSetLayout());
        set->SetPool(pool->GetDescriptorPool());
        m_descriptorSets.push_back(set);
        return set;
    }

}