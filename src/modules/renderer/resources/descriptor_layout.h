//
// Created by lepag on 6/10/2025.
//

#pragma once

#include <vector>
#include <volk.h>

class RenderingDevice;

struct LayoutBinding
{
    uint32_t binding;
    VkDescriptorType bindingType;
    uint32_t count;
    VkShaderStageFlags stage;
    const VkSampler* immutableSamplers;
};

class DescriptorLayout  {
public:
    explicit DescriptorLayout(RenderingDevice& device): m_device(device) {}
    ~DescriptorLayout() { Cleanup(); }

    DescriptorLayout& AddBinding(LayoutBinding binding);
    DescriptorLayout& ClearBindings();

    bool Init();
    void Cleanup();

    [[nodiscard]] VkDescriptorSetLayout GetDescriptorSetLayout() const
    {
        return m_descriptorSetLayout;
    }
private:
    RenderingDevice& m_device;

    VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
    std::vector<LayoutBinding> m_descriptorSetLayoutBindings;

    bool CreateDescriptorSetLayout();
};
