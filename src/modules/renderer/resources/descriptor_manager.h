//
// Created by lepag on 6/10/2025.
//

#pragma once

#include <memory>
#include <vector>

#include "descriptor_layout.h"
#include "descriptor_pool.h"
#include "descriptor_set.h"

namespace GyroEngine::Resources
{
    class DescriptorManager {
    public:
        explicit DescriptorManager(Device::RenderingDevice& device);
        ~DescriptorManager();

        void Cleanup();

        std::shared_ptr<DescriptorPool> CreateDescriptorPool();
        std::shared_ptr<DescriptorLayout> CreateDescriptorLayout();
        std::shared_ptr<DescriptorSet> CreateDescriptorSet(const std::shared_ptr<DescriptorPool>& pool, const std::shared_ptr<DescriptorLayout>& layout);

        [[nodiscard]] std::vector<std::shared_ptr<DescriptorSet>> GetDescriptorSets()
        {
            return m_descriptorSets;
        }

        [[nodiscard]] std::vector<std::shared_ptr<DescriptorPool>> GetDescriptorPools()
        {
            return m_descriptorPools;
        }

        [[nodiscard]] std::vector<std::shared_ptr<DescriptorLayout>> GetDescriptorLayouts()
        {
            return m_descriptorLayouts;
        }
    private:
        Device::RenderingDevice& m_device;

        std::vector<std::shared_ptr<DescriptorPool>> m_descriptorPools;
        std::vector<std::shared_ptr<DescriptorSet>> m_descriptorSets;
        std::vector<std::shared_ptr<DescriptorLayout>> m_descriptorLayouts;
    };

}