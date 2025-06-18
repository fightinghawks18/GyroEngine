//
// Created by lepag on 6/10/2025.
//

#pragma once

#include <vector>
#include <volk.h>
class RenderingDevice;

struct PoolSize
{
    VkDescriptorType type;
    uint32_t maxSets;
};

class DescriptorPool  {
public:
    explicit DescriptorPool(RenderingDevice& device) : m_device(device) {}
    ~DescriptorPool() { Cleanup(); }

    DescriptorPool& SetPoolSize(const std::vector<PoolSize>& poolSizes);
    DescriptorPool& AddPoolSize(PoolSize poolSize);
    DescriptorPool& ClearPoolSizes();

    DescriptorPool& SetMaxSets(uint32_t maxSets);

    bool Init();
    void Cleanup();

    [[nodiscard]] VkDescriptorPool GetDescriptorPool() const
    {
        return m_descriptorPool;
    }
private:
    RenderingDevice& m_device;

    VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;

    uint32_t m_maxSets = 0;
    std::vector<PoolSize> m_descriptorPoolSizes;

    bool CreateDescriptorPool();
};
