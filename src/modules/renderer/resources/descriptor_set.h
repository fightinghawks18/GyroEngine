//
// Created by lepag on 6/10/2025.
//

#pragma once

#include <volk.h>
class RenderingDevice;

struct FrameContext;

class DescriptorSet  {
public:
    explicit DescriptorSet(RenderingDevice& device): m_device(device) {}
    ~DescriptorSet() { Cleanup(); }

    DescriptorSet& SetLayout(VkDescriptorSetLayout layout);
    DescriptorSet& SetPool(VkDescriptorPool pool);

    bool Init();
    void Cleanup();

    void Bind(const FrameContext& frameContext, VkPipelineLayout layout);

    void UpdateBuffer(uint32_t binding, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range) const;
    void UpdateImage(uint32_t binding, VkImageView view, VkSampler sampler) const;
private:
    RenderingDevice& m_device;

    VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_layout = VK_NULL_HANDLE;
    VkDescriptorSet m_descriptorSet = VK_NULL_HANDLE;

    bool CreateDescriptorSet();
};
