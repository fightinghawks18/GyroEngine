//
// Created by lepag on 6/10/2025.
//

#pragma once

#include <volk.h>
#include "device_resource.h"

struct FrameContext;

class DescriptorSet : public IDeviceResource {
public:
    explicit DescriptorSet(RenderingDevice& device): IDeviceResource(device) {}
    ~DescriptorSet() override = default;

    DescriptorSet& setLayout(VkDescriptorSetLayout layout);
    DescriptorSet& setPool(VkDescriptorPool pool);

    bool init() override;
    void cleanup() override;

    void bind(const FrameContext& frameContext, VkPipelineLayout layout);

    void updateBuffer(uint32_t binding, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range) const;
    void updateImage(uint32_t binding, VkImageView view, VkSampler sampler) const;
private:
    VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_layout = VK_NULL_HANDLE;
    VkDescriptorSet m_descriptorSet = VK_NULL_HANDLE;

    bool createDescriptorSet();
};
