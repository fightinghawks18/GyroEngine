//
// Created by lepag on 6/10/2025.
//

#include "pipeline.h"

#include "context/rendering_device.h"

Pipeline& Pipeline::setDescriptorManager(const std::shared_ptr<DescriptorManager>& descriptorManager)
{
    m_descriptorManager = descriptorManager;
    return *this;
}

Pipeline& Pipeline::clearConfig()
{
    m_pipelineConfig = {};
    return *this;
}

bool Pipeline::init()
{
    if (m_pipeline != VK_NULL_HANDLE)
    {
        return false;
    }
    if (!buildPipelineLayout()) return false;
    if (!buildPipeline()) return false;
    return true;
}

void Pipeline::cleanup()
{
    if (m_pipeline != VK_NULL_HANDLE)
    {
        vkDestroyPipeline(m_device.getLogicalDevice(), m_pipeline, nullptr);
        m_pipeline = VK_NULL_HANDLE;
    }

    if (m_pipelineLayout != VK_NULL_HANDLE)
    {
        vkDestroyPipelineLayout(m_device.getLogicalDevice(), m_pipelineLayout, nullptr);
        m_pipelineLayout = VK_NULL_HANDLE;
    }
}

bool Pipeline::buildPipelineLayout()
{
    auto descriptorLayouts = m_descriptorManager->getDescriptorLayouts();

    std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
    descriptorSetLayouts.reserve(descriptorLayouts.size());
    for (auto& descriptorLayout : descriptorLayouts)
    {
        descriptorSetLayouts.push_back(descriptorLayout->getDescriptorSetLayout());
    }

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = descriptorSetLayouts.size();
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();

    // TODO: Create push constants
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;

    if (vkCreatePipelineLayout(m_device.getLogicalDevice(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS)
    {
        Printer::error("Failed to create pipeline layout");
        return false;
    }
    return true;
}

bool Pipeline::buildPipeline()
{
    m_pipelineConfig.pipelineLayout = m_pipelineLayout;

    VkGraphicsPipelineCreateInfo pipelineInfo = pipelineutils::pipelineConfigToVkGraphicsPipeline(m_pipelineConfig);
    VkResult result = vkCreateGraphicsPipelines(m_device.getLogicalDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pipeline);
    if (result != VK_SUCCESS)
    {
        Printer::error("Failed to create pipeline");
        return false;
    }
    return true;
}
