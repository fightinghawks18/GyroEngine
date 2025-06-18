//
// Created by lepag on 6/10/2025.
//

#include "descriptor_set.h"

#include "context/rendering_device.h"
#include "rendering/renderer.h"

DescriptorSet& DescriptorSet::SetLayout(VkDescriptorSetLayout layout)
{
    m_layout = layout;
    return *this;
}

DescriptorSet& DescriptorSet::SetPool(VkDescriptorPool pool)
{
    m_descriptorPool = pool;
    return *this;
}

bool DescriptorSet::Init()
{
    if (m_descriptorSet != VK_NULL_HANDLE)
    {
        return false;
    }
    return CreateDescriptorSet();
}

void DescriptorSet::Cleanup()
{
    if (m_descriptorSet != VK_NULL_HANDLE)
    {
        vkFreeDescriptorSets(m_device.GetLogicalDevice(), m_descriptorPool, 1, &m_descriptorSet);
        m_descriptorSet = VK_NULL_HANDLE;
    }
}

void DescriptorSet::Bind(const FrameContext& frameContext, VkPipelineLayout layout)
{
    if (m_descriptorSet != VK_NULL_HANDLE)
    {
        vkCmdBindDescriptorSets(frameContext.cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, layout,
                        0, 1, &m_descriptorSet, 0, nullptr);
    }
}

void DescriptorSet::UpdateBuffer(uint32_t binding, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range) const
{
    VkDescriptorBufferInfo descriptorBufferInfo = {};
    descriptorBufferInfo.buffer = buffer;
    descriptorBufferInfo.range = range;
    descriptorBufferInfo.offset = offset;

    VkWriteDescriptorSet descriptorWrite = {};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = m_descriptorSet;
    descriptorWrite.dstBinding = binding;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pBufferInfo = &descriptorBufferInfo;

    vkUpdateDescriptorSets(m_device.GetLogicalDevice(), 1, &descriptorWrite, 0, nullptr);
}

void DescriptorSet::UpdateImage(uint32_t binding, VkImageView view, VkSampler sampler) const
{
    VkDescriptorImageInfo descriptorImageInfo = {};
    descriptorImageInfo.sampler = sampler;
    descriptorImageInfo.imageView = view;
    descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet descriptorWrite = {};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = m_descriptorSet;
    descriptorWrite.dstBinding = binding;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo = &descriptorImageInfo;

    vkUpdateDescriptorSets(m_device.GetLogicalDevice(), 1, &descriptorWrite, 0, nullptr);
}

bool DescriptorSet::CreateDescriptorSet()
{
    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
    descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocateInfo.descriptorPool = m_descriptorPool;
    descriptorSetAllocateInfo.descriptorSetCount = 1;
    descriptorSetAllocateInfo.pSetLayouts = &m_layout;

    VkResult result = vkAllocateDescriptorSets(m_device.GetLogicalDevice(), &descriptorSetAllocateInfo, &m_descriptorSet);
    if (result != VK_SUCCESS)
    {
        Printer::LogError("Failed to allocate descriptor sets");
        return false;
    }
    return true;
}
