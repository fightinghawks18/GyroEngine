//
// Created by lepag on 6/8/2025.
//

#pragma once

#include <volk.h>

namespace GyroEngine::Utils::Image
{
    static uint32_t GetSourceAccessMask(VkImageLayout src)
    {
        switch (src) {
            case VK_IMAGE_LAYOUT_UNDEFINED:
                return 0;
            case VK_IMAGE_LAYOUT_PREINITIALIZED:
                return VK_ACCESS_HOST_WRITE_BIT;
            case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
                return VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
                return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
                return VK_ACCESS_TRANSFER_READ_BIT;
            case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
                return VK_ACCESS_TRANSFER_WRITE_BIT;
            case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
                return VK_ACCESS_SHADER_READ_BIT;
            default:
                return 0;
        }
    }


    static uint32_t GetDestinationAccessMask(VkImageLayout dst)
    {
        switch (dst) {
            case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
                return VK_ACCESS_TRANSFER_WRITE_BIT;
            case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
                return VK_ACCESS_TRANSFER_READ_BIT;
            case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
                return VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
                return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
                return VK_ACCESS_SHADER_READ_BIT;
            default:
                return 0;
        }
    }

    static VkPipelineStageFlags GetSourceStageFlags(VkImageLayout src)
    {
        switch (src) {
            case VK_IMAGE_LAYOUT_UNDEFINED:
                return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            case VK_IMAGE_LAYOUT_PREINITIALIZED:
                return VK_PIPELINE_STAGE_HOST_BIT;
            case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
                return VK_PIPELINE_STAGE_TRANSFER_BIT;
            case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
                return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
                return VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
            case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
                return VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
            default:
                return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        }
    }

    static VkPipelineStageFlags GetDestinationStageFlags(VkImageLayout dst)
    {
        switch (dst) {
            case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
                return VK_PIPELINE_STAGE_TRANSFER_BIT;
            case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
                return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
                return VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
            case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
                return VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
            default:
                return VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        }
    }
}