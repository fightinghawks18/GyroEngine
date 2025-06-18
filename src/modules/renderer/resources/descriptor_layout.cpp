//
// Created by lepag on 6/10/2025.
//

#include "descriptor_layout.h"

#include "context/rendering_device.h"

namespace GyroEngine::Resources
{
    DescriptorLayout& DescriptorLayout::AddBinding(const LayoutBinding &binding)
    {
        m_descriptorSetLayoutBindings.push_back(binding);
        return *this;
    }

    DescriptorLayout& DescriptorLayout::ClearBindings()
    {
        m_descriptorSetLayoutBindings.clear();
        return *this;
    }

    bool DescriptorLayout::Init()
    {
        if (m_descriptorSetLayout != VK_NULL_HANDLE)
        {
            return false;
        }
        return CreateDescriptorSetLayout();
    }

    void DescriptorLayout::Cleanup()
    {
        if (m_descriptorSetLayout != VK_NULL_HANDLE)
        {
            vkDestroyDescriptorSetLayout(m_device.GetLogicalDevice(), m_descriptorSetLayout, nullptr);
            m_descriptorSetLayout = VK_NULL_HANDLE;
        }
    }

    bool DescriptorLayout::CreateDescriptorSetLayout()
    {
        std::vector<VkDescriptorSetLayoutBinding> bindings;
        bindings.reserve(m_descriptorSetLayoutBindings.size());
        for (const auto&[binding, bindingType, count, stage, immutableSamplers] : m_descriptorSetLayoutBindings)
        {
            bindings.push_back({binding, bindingType, count, stage, immutableSamplers});
        }

        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo = {};
        descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        descriptorSetLayoutInfo.pBindings = bindings.data();

        VkResult result = vkCreateDescriptorSetLayout(m_device.GetLogicalDevice(), &descriptorSetLayoutInfo, nullptr, &m_descriptorSetLayout);
        if (result != VK_SUCCESS)
        {
            Printer::LogError("Failed to create descriptor set layout");
            return false;
        }
        return true;
    }

}