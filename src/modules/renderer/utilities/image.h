//
// Created by lepag on 6/8/2025.
//

#pragma once

#include <volk.h>

#include "implementation/stb_implementation.h"

namespace GyroEngine::Utils::Image
{
    struct ImageData
    {
        int width;
        int height;
        int channels;
        void* data;
    };

    static ImageData* LoadImageData(const std::string& filePath, const int width, const int height)
{
    stbi_set_flip_vertically_on_load(true);
    const int outputChannels = 4; // Always force RGBA

    auto imageData = new ImageData();
    imageData->data = stbi_load(filePath.c_str(), &imageData->width, &imageData->height, &imageData->channels, outputChannels);

    if (!imageData->data)
    {
        delete imageData;
        Logger::LogError("Failure to load image at path {} because: {}", filePath, stbi_failure_reason());
        return nullptr;
    }

    if (imageData->channels < 3 || imageData->channels > 4)
    {
        stbi_image_free(const_cast<void*>(imageData->data));
        delete imageData;
        Logger::LogError("Invalid amount of channels in image: {} at path {}", imageData->channels, filePath);
        return nullptr; // Unsupported number of channels
    }

    // Always use outputChannels for buffer size and stbir_resize!
    const bool shouldResize = (width > 0 && height > 0) &&
                (imageData->width != width || imageData->height != height);
    if (shouldResize)
    {
        // Allocate buffer for resized image
        auto resizedData = static_cast<unsigned char *>(malloc(width * height * outputChannels));
        if (!resizedData)
        {
            stbi_image_free(const_cast<void*>(imageData->data));
            delete imageData;
            Logger::LogError("Failed to allocate memory for resized image at path: {}", filePath);
            return nullptr;
        }

        int inputStride = imageData->width * outputChannels;
        int outputStride = width * outputChannels;
        stbir_pixel_layout layout = STBIR_RGBA;

        unsigned char* result = stbir_resize_uint8_srgb(
            static_cast<const unsigned char*>(imageData->data),
            imageData->width,
            imageData->height,
            inputStride,
            resizedData,
            width,
            height,
            outputStride,
            layout
        );

        stbi_image_free(imageData->data);

        if (!result)
        {
            free(resizedData);
            delete imageData;
            Logger::LogError("Failed to resize image at path: {}", filePath);
            return nullptr;
        }

        imageData->data = resizedData;
        imageData->width = width;
        imageData->height = height;
        imageData->channels = outputChannels;
    }
    else
    {
        imageData->channels = outputChannels; // track actual channel count used
    }

    return imageData;
}

    static VkFormat GetFormatFromChannels(int channels)
    {
        switch (channels) {
            case 1:
                return VK_FORMAT_R8_UNORM;
            case 2:
                return VK_FORMAT_R8G8_UNORM;
            case 3:
            case 4:
                return VK_FORMAT_R8G8B8A8_UNORM;
            default:
                throw std::runtime_error("Unsupported number of channels: " + std::to_string(channels));
        }
    }

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