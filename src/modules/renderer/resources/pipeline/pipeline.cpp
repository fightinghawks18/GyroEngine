//
// Created by lepag on 6/10/2025.
//

#include "pipeline.h"

#include <map>

#include "context/rendering_device.h"
#include "rendering/renderer.h"

namespace GyroEngine::Resources
{
    Pipeline &Pipeline::SetPipelineBindings(const std::shared_ptr<PipelineBindings> &pipelineBindings)
    {
        m_pipelineBindings = pipelineBindings;
        return *this;
    }

    Pipeline &Pipeline::ClearConfig()
    {
        m_pipelineConfig = {};
        return *this;
    }

    Pipeline &Pipeline::SetColorFormat(VkFormat colorFormat)
    {
        m_pipelineConfig.colorFormat = colorFormat;
        return *this;
    }


    bool Pipeline::Init()
    {
        if (m_pipeline != VK_NULL_HANDLE)
        {
            return false;
        }
        if (!BuildPipelineLayout()) return false;
        if (!BuildPipeline()) return false;
        return true;
    }

    void Pipeline::Cleanup()
    {
        m_device.WaitForIdle();
        if (m_pipeline != VK_NULL_HANDLE)
        {
            vkDestroyPipeline(m_device.GetLogicalDevice(), m_pipeline, nullptr);
            m_pipeline = VK_NULL_HANDLE;
        }

        if (m_pipelineLayout != VK_NULL_HANDLE)
        {
            vkDestroyPipelineLayout(m_device.GetLogicalDevice(), m_pipelineLayout, nullptr);
            m_pipelineLayout = VK_NULL_HANDLE;
        }
    }

    void Pipeline::Bind(const Rendering::FrameContext &frameContext) const
    {
        if (m_pipeline == VK_NULL_HANDLE || m_pipelineLayout == VK_NULL_HANDLE)
            return;

        vkCmdBindPipeline(frameContext.cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

        if (!m_pipelineBindings)
            return;

        m_pipelineBindings->Bind(frameContext, m_pipelineLayout);
    }

    void Pipeline::DrawFullscreenQuad(const Rendering::FrameContext &frameContext) const
    {
        VkCommandBuffer cmd = frameContext.cmd;
        if (m_pipelineConfig.inputAssemblyState.topology == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
        {
            vkCmdDraw(cmd, 6, 1, 0, 0); // Draw quad as 2 triangles
        } else
        {
            vkCmdDraw(cmd, 4, 1, 0, 0); // Draw quad as triangle strip
        }
    }

    bool Pipeline::BuildPipelineLayout()
    {
        VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;


        m_descriptorSetLayouts.clear();
        std::map<uint32_t, VkDescriptorSetLayout> descriptorSetLayouts;

        // Fill the map
        for (const auto set : m_pipelineBindings->GetSets()) {
            descriptorSetLayouts[set->set] = set->layout;
        }

        // Fill the vector in sorted order of set index
        for (const auto& [setIndex, layout] : descriptorSetLayouts) {
            m_descriptorSetLayouts.push_back(layout);
        }

        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(m_descriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = m_descriptorSetLayouts.data();

        const auto pushConstants = m_pipelineBindings->GetPushConstants();

        if (!pushConstants.empty())
        {
            m_pushConstantRanges.clear();
            m_pushConstantRanges.reserve(pushConstants.size());
            for (const auto &pushConstant: pushConstants)
            {
                VkPushConstantRange pushConstantRange = {};
                pushConstantRange.stageFlags = pushConstant.stageFlags;
                pushConstantRange.offset = pushConstant.offset;
                pushConstantRange.size = pushConstant.size;
                m_pushConstantRanges.push_back(pushConstantRange);
            }
            pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(m_pushConstantRanges.size());
            pipelineLayoutInfo.pPushConstantRanges = m_pushConstantRanges.data();
        } else
        {
            pipelineLayoutInfo.pushConstantRangeCount = 0;
            pipelineLayoutInfo.pPushConstantRanges = nullptr;
        }

        if (vkCreatePipelineLayout(m_device.GetLogicalDevice(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) !=
            VK_SUCCESS)
        {
            Logger::LogError("Failed to create pipeline layout");
            return false;
        }
        return true;
    }

    bool Pipeline::BuildPipeline()
    {
        m_pipelineConfig.pipelineLayout = m_pipelineLayout;

        VkGraphicsPipelineCreateInfo pipelineInfo = {};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

        // Shader stages
        std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
        shaderStages.reserve(m_pipelineConfig.shaderStages.size());
        for (const auto &[stage, module, entryPoint]: m_pipelineConfig.shaderStages)
        {
            VkPipelineShaderStageCreateInfo shaderStageInfo = {};
            shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shaderStageInfo.stage = stage;
            shaderStageInfo.module = module;
            shaderStageInfo.pName = entryPoint;
            shaderStages.push_back(shaderStageInfo);
        }

        pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
        pipelineInfo.pStages = shaderStages.data();

        // Vertex input
        std::vector<VkVertexInputAttributeDescription> vertexInputAttributeDescriptions;
        std::vector<VkVertexInputBindingDescription> vertexInputBindingDescriptions;

        uint32_t attributeCount = 0;
        for (const auto& binding : m_pipelineConfig.vertexInputState.inputBindings)
        {
            attributeCount += static_cast<uint32_t>(binding.inputAttributes.size());
        }

        vertexInputBindingDescriptions.reserve(m_pipelineConfig.vertexInputState.inputBindings.size());
        vertexInputAttributeDescriptions.reserve(attributeCount);

        for (const auto &[binding, stride, inputRate, attributes]: m_pipelineConfig.vertexInputState.inputBindings)
        {
            VkVertexInputBindingDescription vertexInputBindingDescription = {};
            vertexInputBindingDescription.binding = binding;
            vertexInputBindingDescription.stride = stride;
            vertexInputBindingDescription.inputRate = inputRate;
            vertexInputBindingDescriptions.push_back(vertexInputBindingDescription);

            for (const auto& [name, offset] : attributes)
            {
                PipelineBindings::VertexInput inputAttr = {};
                for (const auto& attr : m_pipelineBindings->GetVertexInputs())
                {
                    if (attr.name == name)
                    {
                        inputAttr = attr;
                        break;
                    }
                }
                if (inputAttr.name.empty())
                {
                    Logger::LogError("Failed to find input attribute with name {}", name);;
                    return false;
                }
                VkVertexInputAttributeDescription vertexInputAttributeDescription = {};
                vertexInputAttributeDescription.binding = binding;
                vertexInputAttributeDescription.location = inputAttr.location;
                vertexInputAttributeDescription.format = inputAttr.format;
                vertexInputAttributeDescription.offset = offset;
                vertexInputAttributeDescriptions.push_back(vertexInputAttributeDescription);
            }
        }

        VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexInputBindingDescriptions.size());
        vertexInputInfo.pVertexBindingDescriptions = vertexInputBindingDescriptions.data();
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInputAttributeDescriptions.
            size());
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
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
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
        VkFormat depthFormat = m_device.GetPreferredDepthFormat();
        VkFormat stencilFormat = m_device.GetPreferredStencilFormat();

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
        for (const auto &colorBlendState: m_pipelineConfig.colorBlendState.colorBlendStates)
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
        colorBlending.attachmentCount = static_cast<uint32_t>(colorBlendAttachments.size());
        colorBlending.pAttachments = colorBlendAttachments.data();

        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisample;
        pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.pNext = &renderingInfo;

        pipelineInfo.layout = m_pipelineConfig.pipelineLayout;
        pipelineInfo.renderPass = VK_NULL_HANDLE;
        pipelineInfo.subpass = 0;

        VkResult result = vkCreateGraphicsPipelines(m_device.GetLogicalDevice(), VK_NULL_HANDLE, 1, &pipelineInfo,
                                                    nullptr,
                                                    &m_pipeline);
        if (result != VK_SUCCESS)
        {
            Logger::LogError("Failed to create pipeline");
            return false;
        }
        return true;
    }
}
