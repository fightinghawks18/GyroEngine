//
// Created by lepag on 6/10/2025.
//

#include "descriptor_set.h"

#include "context/rendering_device.h"

DescriptorSet& DescriptorSet::setLayout(VkDescriptorSetLayout layout)
{
    m_layout = layout;
    return *this;
}

DescriptorSet& DescriptorSet::setPool(VkDescriptorPool pool)
{
    m_descriptorPool = pool;
    return *this;
}

bool DescriptorSet::init()
{
    if (m_descriptorSet != VK_NULL_HANDLE)
    {
        return false;
    }
    return createDescriptorSet();
}

void DescriptorSet::cleanup()
{
    if (m_descriptorSet != VK_NULL_HANDLE)
    {
        vkFreeDescriptorSets(m_device.getLogicalDevice(), m_descriptorPool, 1, &m_descriptorSet);
        m_descriptorSet = VK_NULL_HANDLE;
    }
}

void DescriptorSet::updateBuffer(uint32_t binding, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range) const
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

    vkUpdateDescriptorSets(m_device.getLogicalDevice(), 1, &descriptorWrite, 0, nullptr);
}

void DescriptorSet::updateImage(uint32_t binding, VkImageView view, VkSampler sampler) const
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

    vkUpdateDescriptorSets(m_device.getLogicalDevice(), 1, &descriptorWrite, 0, nullptr);
}

bool DescriptorSet::createDescriptorSet()
{
    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
    descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocateInfo.descriptorPool = m_descriptorPool;
    descriptorSetAllocateInfo.descriptorSetCount = 1;

    VkResult result = vkAllocateDescriptorSets(m_device.getLogicalDevice(), &descriptorSetAllocateInfo, &m_descriptorSet);
    if (result != VK_SUCCESS)
    {
        Printer::error("Failed to allocate descriptor sets");
        return false;
    }
    return true;
}
