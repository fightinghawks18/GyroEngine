//
// Created by lepag on 6/10/2025.
//

#pragma once
#include "rendering/viewport.h"
#include "resources/pipeline.h"

// TODO: Finish pipeline helper
namespace pipelineutils
{
    struct PipelineInputAttribute
    {
        uint32_t binding = 0;
        uint32_t location = 0;
        uint32_t stride = 0;
        uint32_t offset = 0;
        VkVertexInputRate inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        VkFormat format = VK_FORMAT_UNDEFINED;
    };

    struct PipelineInputState
    {
        std::vector<PipelineInputAttribute> inputAttributes;

        void addAttribute(uint32_t binding, uint32_t location, uint32_t stride, uint32_t offset,
                          VkVertexInputRate inputRate, VkFormat format)
        {
            inputAttributes.push_back({binding, location, stride, offset, inputRate, format});
        }
    };

    struct PipelineInputAssemblyState
    {
        VkPrimitiveTopology topology;
        VkBool32 primitiveRestartEnable;
    };

    struct PipelineViewportState
    {
        std::vector<VkViewport> viewports;
        std::vector<VkRect2D> scissors;
    };

    struct PipelineRasterizerState
    {
        VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL;
        VkCullModeFlags cullMode = VK_CULL_MODE_BACK_BIT;
        VkFrontFace frontFace = VK_FRONT_FACE_CLOCKWISE;
    };

    struct PipelineMultisampleState
    {
        VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
        VkBool32 sampleShading = VK_FALSE;
    };

    struct PipelineDepthStencilState
    {
        VkBool32 depthTest = VK_TRUE;
        VkBool32 depthWrite = VK_TRUE;
        VkCompareOp depthCompareOp = VK_COMPARE_OP_LESS;
    };

    struct PipelineBlendState
    {
        VkBool32 blendEnable = VK_FALSE;
        VkBlendFactor srcBlendFactor = VK_BLEND_FACTOR_ONE;
        VkBlendFactor dstBlendFactor = VK_BLEND_FACTOR_ZERO;
        VkBlendOp srcBlendOp = VK_BLEND_OP_ADD;
    };

    struct PipelinePushConstant
    {
        VkShaderStageFlags shaderStage = 0;
        uint32_t offset = 0;
        uint32_t size = 0;
    };

    struct PipelineColorBlendAttachment
    {
        VkColorComponentFlags colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        VkBool32 blendEnable = VK_FALSE;
        VkBlendFactor srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        VkBlendFactor dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
        VkBlendOp colorBlendOp = VK_BLEND_OP_ADD;
        VkBlendFactor srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        VkBlendFactor dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        VkBlendOp alphaBlendOp = VK_BLEND_OP_ADD;
    };

    struct PipelineColorBlendState
    {
        std::vector<PipelineColorBlendAttachment> colorBlendStates;

        void addAttachment()
        {
            colorBlendStates.push_back({});
        }
    };

    struct PipelineShaderStage
    {
        VkShaderStageFlagBits stage = VK_SHADER_STAGE_VERTEX_BIT;
        VkShaderModule module = VK_NULL_HANDLE;
        const char* entryPoint = "main";
    };

    struct PipelineConfig
    {
        std::vector<PipelineShaderStage> shaderStages;
        PipelineInputState vertexInputState;
        PipelineInputAssemblyState inputAssemblyState;
        PipelineRasterizerState rasterizerState;
        PipelineDepthStencilState depthStencilState;
        PipelineViewportState viewportState;
        PipelineMultisampleState multisampleState;
        PipelineColorBlendState colorBlendState;
        PipelineBlendState blendState;
        std::vector<PipelinePushConstant> pushConstants;
        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    };

    static VkGraphicsPipelineCreateInfo pipelineConfigToVkGraphicsPipeline(const PipelineConfig& config)
    {
        VkGraphicsPipelineCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

        // Shader stages
        std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
        shaderStages.reserve(config.shaderStages.size());
        for (const auto& shaderStage : config.shaderStages)
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

        vertexInputBindingDescriptions.reserve(config.vertexInputState.inputAttributes.size());
        vertexInputAttributeDescriptions.reserve(config.vertexInputState.inputAttributes.size());
        for (const auto& inputAttribute : config.vertexInputState.inputAttributes)
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
        inputAssembly.topology = config.inputAssemblyState.topology;
        inputAssembly.primitiveRestartEnable = config.inputAssemblyState.primitiveRestartEnable;

        std::vector<VkViewport> viewports;
        viewports.reserve(config.viewportState.viewports.size());
        for (const auto& viewport : config.viewportState.viewports)
        {
            viewports.push_back(viewport);
        }
        std::vector<VkRect2D> scissors;
        scissors.reserve(config.viewportState.scissors.size());
        for (const auto& scissor : config.viewportState.scissors)
        {
            scissors.push_back(scissor);
        }

        std::vector<VkDynamicState> dynamicStates;
        dynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
        dynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);

        VkPipelineDynamicStateCreateInfo dynamicState = {};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = dynamicStates.size();
        dynamicState.pDynamicStates = dynamicStates.data();

        VkPipelineViewportStateCreateInfo viewportState = {};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = viewports.size();
        viewportState.pViewports = viewports.data();
        viewportState.scissorCount = scissors.size();
        viewportState.pScissors = scissors.data();

        VkPipelineRasterizationStateCreateInfo rasterizer = {};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = config.rasterizerState.polygonMode;
        rasterizer.cullMode = config.rasterizerState.cullMode;
        rasterizer.frontFace = config.rasterizerState.frontFace;
        rasterizer.lineWidth = 1.0f;
        rasterizer.depthBiasEnable = VK_FALSE;

        VkPipelineMultisampleStateCreateInfo multisample = {};
        multisample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisample.sampleShadingEnable = config.multisampleState.sampleShading;
        multisample.rasterizationSamples = config.multisampleState.samples;

        VkPipelineDepthStencilStateCreateInfo depthStencil = {};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = config.depthStencilState.depthTest;
        depthStencil.depthWriteEnable = config.depthStencilState.depthWrite;
        depthStencil.depthCompareOp = config.depthStencilState.depthCompareOp;
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.stencilTestEnable = VK_FALSE;

        std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments;
        colorBlendAttachments.reserve(config.colorBlendState.colorBlendStates.size());
        for (const auto& colorBlendState : config.colorBlendState.colorBlendStates)
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

        info.layout = config.pipelineLayout;
        info.renderPass = VK_NULL_HANDLE;
        info.subpass = 0;
        return info;


    }
}
