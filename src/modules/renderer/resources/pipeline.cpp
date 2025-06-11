//
// Created by lepag on 6/10/2025.
//

#include "pipeline.h"

#include "context/rendering_device.h"
#include "rendering/renderer.h"

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

Pipeline& Pipeline::setColorFormat(VkFormat colorFormat)
{
    m_pipelineConfig.colorFormat = colorFormat;
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

void Pipeline::bind(const FrameContext& frameContext)
{
    if (m_pipeline != VK_NULL_HANDLE)
    {
        vkCmdBindPipeline(frameContext.cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
    }

    if (m_descriptorManager)
    {
        auto descriptorSets = m_descriptorManager->getDescriptorSets();
        for (auto& descriptorSet : descriptorSets)
        {
            descriptorSet->bind(frameContext, m_pipelineLayout);
        }
    }
}

bool Pipeline::buildPipelineLayout()
{
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    if (m_descriptorManager)
    {
        auto descriptorLayouts = m_descriptorManager->getDescriptorLayouts();

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
        descriptorSetLayouts.reserve(descriptorLayouts.size());
        for (auto& descriptorLayout : descriptorLayouts)
        {
            descriptorSetLayouts.push_back(descriptorLayout->getDescriptorSetLayout());
        }

        pipelineLayoutInfo.setLayoutCount = descriptorSetLayouts.size();
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
    }
    else
    {
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pSetLayouts = nullptr;
    }

    // TODO: Create push constants
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;

    if (vkCreatePipelineLayout(m_device.getLogicalDevice(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) !=
        VK_SUCCESS)
    {
        Printer::error("Failed to create pipeline layout");
        return false;
    }
    return true;
}

bool Pipeline::buildPipeline()
{
    m_pipelineConfig.pipelineLayout = m_pipelineLayout;

    VkGraphicsPipelineCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

    // Shader stages
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
    shaderStages.reserve(m_pipelineConfig.shaderStages.size());
    for (const auto& shaderStage : m_pipelineConfig.shaderStages)
    {
        VkPipelineShaderStageCreateInfo shaderStageInfo = {};
        shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStageInfo.stage = shaderStage.stage;
        shaderStageInfo.module = shaderStage.module;
        shaderStageInfo.pName = shaderStage.entryPoint;
        shaderStages.push_back(shaderStageInfo);
    }

    info.stageCount = shaderStages.size();
    info.pStages = shaderStages.data();

    // Vertex input
    std::vector<VkVertexInputAttributeDescription> vertexInputAttributeDescriptions;
    std::vector<VkVertexInputBindingDescription> vertexInputBindingDescriptions;

    vertexInputBindingDescriptions.reserve(m_pipelineConfig.vertexInputState.inputAttributes.size());
    vertexInputAttributeDescriptions.reserve(m_pipelineConfig.vertexInputState.inputAttributes.size());
    for (const auto& inputAttribute : m_pipelineConfig.vertexInputState.inputAttributes)
    {
        VkVertexInputAttributeDescription vertexInputAttributeDescription = {};
        vertexInputAttributeDescription.binding = inputAttribute.binding;
        vertexInputAttributeDescription.location = inputAttribute.location;
        vertexInputAttributeDescription.format = inputAttribute.format;
        vertexInputAttributeDescription.offset = inputAttribute.offset;
        vertexInputAttributeDescriptions.push_back(vertexInputAttributeDescription);

        VkVertexInputBindingDescription bindingDescription = {};
        bindingDescription.binding = inputAttribute.binding;
        bindingDescription.stride = inputAttribute.stride;
        bindingDescription.inputRate = inputAttribute.inputRate;
        vertexInputBindingDescriptions.push_back(bindingDescription);
    }

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = vertexInputBindingDescriptions.size();
    vertexInputInfo.pVertexBindingDescriptions = vertexInputBindingDescriptions.data();
    vertexInputInfo.vertexAttributeDescriptionCount = vertexInputAttributeDescriptions.size();
    vertexInputInfo.pVertexAttributeDescriptions = vertexInputAttributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = m_pipelineConfig.inputAssemblyState.topology;
    inputAssembly.primitiveRestartEnable = m_pipelineConfig.inputAssemblyState.primitiveRestartEnable;

    std::vector<VkDynamicState> dynamicStates;
    dynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
    dynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);

    VkPipelineDynamicStateCreateInfo dynamicState = {};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = dynamicStates.size();
    dynamicState.pDynamicStates = dynamicStates.data();

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;
    viewportState.pViewports = nullptr;
    viewportState.pScissors = nullptr;

    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = m_pipelineConfig.rasterizerState.polygonMode;
    rasterizer.cullMode = m_pipelineConfig.rasterizerState.cullMode;
    rasterizer.frontFace = m_pipelineConfig.rasterizerState.frontFace;
    rasterizer.lineWidth = 1.0f;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkFormat colorFormat = m_pipelineConfig.colorFormat;
    VkFormat depthFormat = m_device.getPreferredDepthFormat();
    VkFormat stencilFormat = m_device.getPreferredStencilFormat();

    VkPipelineRenderingCreateInfoKHR renderingInfo = {};
    renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachmentFormats = &colorFormat;
    renderingInfo.depthAttachmentFormat = depthFormat;
    renderingInfo.stencilAttachmentFormat = stencilFormat;

    VkPipelineMultisampleStateCreateInfo multisample = {};
    multisample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisample.sampleShadingEnable = m_pipelineConfig.multisampleState.sampleShading;
    multisample.rasterizationSamples = m_pipelineConfig.multisampleState.samples;

    VkPipelineDepthStencilStateCreateInfo depthStencil = {};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = m_pipelineConfig.depthStencilState.depthTest;
    depthStencil.depthWriteEnable = m_pipelineConfig.depthStencilState.depthWrite;
    depthStencil.depthCompareOp = m_pipelineConfig.depthStencilState.depthCompareOp;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;

    std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments;
    colorBlendAttachments.reserve(m_pipelineConfig.colorBlendState.colorBlendStates.size());
    for (const auto& colorBlendState : m_pipelineConfig.colorBlendState.colorBlendStates)
    {
        VkPipelineColorBlendAttachmentState colorBlendAttachment;
        colorBlendAttachment.colorWriteMask = colorBlendState.colorWriteMask;
        colorBlendAttachment.blendEnable = colorBlendState.blendEnable;
        colorBlendAttachment.colorBlendOp = colorBlendState.colorBlendOp;
        colorBlendAttachment.srcColorBlendFactor = colorBlendState.srcColorBlendFactor;
        colorBlendAttachment.dstColorBlendFactor = colorBlendState.dstColorBlendFactor;
        colorBlendAttachment.srcAlphaBlendFactor = colorBlendState.srcAlphaBlendFactor;
        colorBlendAttachment.dstAlphaBlendFactor = colorBlendState.dstAlphaBlendFactor;
        colorBlendAttachment.alphaBlendOp = colorBlendState.alphaBlendOp;
        colorBlendAttachments.push_back(colorBlendAttachment);
    }

    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = colorBlendAttachments.size();
    colorBlending.pAttachments = colorBlendAttachments.data();

    info.pViewportState = &viewportState;
    info.pRasterizationState = &rasterizer;
    info.pMultisampleState = &multisample;
    info.pDepthStencilState = &depthStencil;
    info.pColorBlendState = &colorBlending;
    info.pVertexInputState = &vertexInputInfo;
    info.pInputAssemblyState = &inputAssembly;
    info.pDynamicState = &dynamicState;
    info.pNext = &renderingInfo;

    info.layout = m_pipelineConfig.pipelineLayout;
    info.renderPass = VK_NULL_HANDLE;
    info.subpass = 0;

    VkResult result = vkCreateGraphicsPipelines(m_device.getLogicalDevice(), VK_NULL_HANDLE, 1, &info, nullptr,
                                                &m_pipeline);
    if (result != VK_SUCCESS)
    {
        Printer::error("Failed to create pipeline");
        return false;
    }
    return true;
}
