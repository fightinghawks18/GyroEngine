//
// Created by lepag on 6/10/2025.
//

#pragma once

#include <vector>
#include <volk.h>

#include "device_resource.h"

struct LayoutBinding
{
    uint32_t binding;
    VkDescriptorType bindingType;
    uint32_t count;
    VkShaderStageFlags stage;
    const VkSampler* immutableSamplers;
};

class DescriptorLayout : public IDeviceResource {
public:
    explicit DescriptorLayout(RenderingDevice& device): IDeviceResource(device) {}
    ~DescriptorLayout() override = default;

    DescriptorLayout& addBinding(const LayoutBinding binding);
    DescriptorLayout& clearBindings();

    bool init() override;
    void cleanup() override;

    [[nodiscard]] VkDescriptorSetLayout getDescriptorSetLayout() const
    {
        return m_descriptorSetLayout;
    }
private:
    VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
    std::vector<LayoutBinding> m_descriptorSetLayoutBindings;

    bool createDescriptorSetLayout();
};
