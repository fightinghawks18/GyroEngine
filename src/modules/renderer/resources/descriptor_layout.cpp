//
// Created by lepag on 6/10/2025.
//

#include "descriptor_layout.h"

#include "context/rendering_device.h"

DescriptorLayout& DescriptorLayout::addBinding(const LayoutBinding binding)
{
    m_descriptorSetLayoutBindings.push_back(binding);
    return *this;
}

DescriptorLayout& DescriptorLayout::clearBindings()
{
    m_descriptorSetLayoutBindings.clear();
    return *this;
}

bool DescriptorLayout::init()
{
    if (m_descriptorSetLayout != VK_NULL_HANDLE)
    {
        return false;
    }
    return createDescriptorSetLayout();
}

void DescriptorLayout::cleanup()
{
    if (m_descriptorSetLayout != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorSetLayout(m_device.getLogicalDevice(), m_descriptorSetLayout, nullptr);
        m_descriptorSetLayout = VK_NULL_HANDLE;
    }
}

bool DescriptorLayout::createDescriptorSetLayout()
{
    std::vector<VkDescriptorSetLayoutBinding> bindings;
    bindings.reserve(m_descriptorSetLayoutBindings.size());
    for (const auto& binding : m_descriptorSetLayoutBindings)
    {
        bindings.push_back({binding.binding, binding.bindingType, binding.count, binding.stage, binding.immutableSamplers});
    }

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo = {};
    descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    descriptorSetLayoutInfo.pBindings = bindings.data();

    VkResult result = vkCreateDescriptorSetLayout(m_device.getLogicalDevice(), &descriptorSetLayoutInfo, nullptr, &m_descriptorSetLayout);
    if (result != VK_SUCCESS)
    {
        Printer::error("Failed to create descriptor set layout");
        return false;
    }
    return true;
}
