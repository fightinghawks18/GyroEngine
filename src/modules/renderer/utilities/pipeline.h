//
// Created by lepag on 6/10/2025.
//

#pragma once
#include <vector>

#include "rendering/viewport.h"

namespace GyroEngine::Utils::Pipeline
{
    struct PipelineInputAttribute
    {
        uint32_t binding = 0;
        uint32_t location = 0;
        uint32_t offset = 0;
        VkFormat format = VK_FORMAT_UNDEFINED;
    };

    struct PipelineInputBinding
    {
        uint32_t binding = 0;
        uint32_t stride = 0;
        VkVertexInputRate inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    };

    struct PipelineInputState
    {
        std::vector<PipelineInputAttribute> inputAttributes;
        std::vector<PipelineInputBinding> inputBindings;

        void addAttribute(const uint32_t binding, const uint32_t location, const uint32_t offset, VkFormat format)
        {
            inputAttributes.push_back({binding, location, offset, format});
        }

        void addBinding(const uint32_t binding, const uint32_t stride, VkVertexInputRate inputRate)
        {
            inputBindings.push_back({binding, stride, inputRate});
        }
    };

    struct PipelineInputAssemblyState
    {
        VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        VkBool32 primitiveRestartEnable = VK_FALSE;
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
        const char *entryPoint = "main";
    };

    struct PipelineConfig
    {
        std::vector<PipelineShaderStage> shaderStages{};
        PipelineInputState vertexInputState{};
        PipelineInputAssemblyState inputAssemblyState{};
        PipelineRasterizerState rasterizerState{};
        PipelineDepthStencilState depthStencilState{};
        PipelineViewportState viewportState{};
        PipelineMultisampleState multisampleState{};
        PipelineColorBlendState colorBlendState{};
        PipelineBlendState blendState{};
        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
        VkFormat colorFormat = VK_FORMAT_UNDEFINED;
    };

    inline VkWriteDescriptorSet MakeWriteDescriptorImage(
        VkDescriptorSet dstSet,
        uint32_t binding,
        VkDescriptorType type,
        const VkDescriptorImageInfo *imageInfo,
        uint32_t count = 1,
        uint32_t arrayElement = 0)
    {
        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = dstSet;
        write.dstBinding = binding;
        write.dstArrayElement = arrayElement;
        write.descriptorType = type;
        write.descriptorCount = count;
        write.pImageInfo = imageInfo;
        return write;
    }

    inline VkWriteDescriptorSet MakeWriteDescriptorBuffer(
        VkDescriptorSet dstSet,
        uint32_t binding,
        VkDescriptorType type,
        const VkDescriptorBufferInfo *bufferInfo,
        uint32_t count = 1,
        uint32_t arrayElement = 0)
    {
        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = dstSet;
        write.dstBinding = binding;
        write.dstArrayElement = arrayElement;
        write.descriptorType = type;
        write.descriptorCount = count;
        write.pBufferInfo = bufferInfo;
        return write;
    }
}
