//
// Created by lepag on 6/10/2025.
//

#pragma once

#include <vector>
#include <volk.h>
namespace GyroEngine::Device
{
    class RenderingDevice;
}
using namespace GyroEngine;

struct PoolSize
{
    VkDescriptorType type;
    uint32_t maxSets;
};

namespace GyroEngine::Resources
{
    class DescriptorPool  {
    public:
        explicit DescriptorPool(Device::RenderingDevice& device) : m_device(device) {}
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
        Device::RenderingDevice& m_device;

        VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;

        uint32_t m_maxSets = 0;
        std::vector<PoolSize> m_descriptorPoolSizes;

        bool CreateDescriptorPool();
    };

}