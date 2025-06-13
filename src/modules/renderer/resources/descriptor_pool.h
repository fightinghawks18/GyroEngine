//
// Created by lepag on 6/10/2025.
//

#pragma once

#include <vector>
#include <volk.h>
#include "device_resource.h"

struct PoolSize
{
    VkDescriptorType type;
    uint32_t maxSets;
};

class DescriptorPool : public IDeviceResource {
public:
    explicit DescriptorPool(RenderingDevice& device) : IDeviceResource(device) {}
    ~DescriptorPool() override { DescriptorPool::cleanup(); }

    DescriptorPool& setPoolSize(const std::vector<PoolSize>& poolSizes);
    DescriptorPool& addPoolSize(PoolSize poolSize);
    DescriptorPool& clearPoolSizes();

    bool init() override;
    void cleanup() override;

    [[nodiscard]] VkDescriptorPool getDescriptorPool() const
    {
        return m_descriptorPool;
    }
private:
    VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;

    std::vector<PoolSize> m_descriptorPoolSizes;

    bool createDescriptorPool();
};
