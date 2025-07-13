//
// Created by lepag on 6/8/2025.
//

#pragma once

#include <memory>
#include <volk.h>

#include "../../implementation/vma_implementation.h"
#include "utilities/device.h"

#include "utilities/image.h"

namespace GyroEngine::Device
{
    class RenderingDevice;
}

using namespace GyroEngine;

namespace GyroEngine::Resources
{
    class Image
    {
    public:
        explicit Image(Device::RenderingDevice &device) : m_device(device)
        {
        }

        ~Image() { Cleanup(); }

        Image &SetFormat(VkFormat format);

        Image &SetExtent(VkExtent3D extent);

        Image &SetMipLevels(uint32_t mipLevels);

        Image &SetArrayLayers(uint32_t arrayLayers);

        Image &SetUsage(VkImageUsageFlags usage);

        Image &SetAspectMask(VkImageAspectFlags aspectMask);

        Image &SetImageType(VkImageType imageType);

        Image &SetViewType(VkImageViewType viewType);

        Image &SetSamples(VkSampleCountFlagBits samples);

        Image &SetTiling(VkImageTiling tiling);

        Image &SetCreateFlags(VkImageCreateFlags createFlags);

        Image &SetInitialLayout(VkImageLayout initialLayout);

        bool Init(VkImage externalImage = VK_NULL_HANDLE, VkImageView imageView = VK_NULL_HANDLE);

        void Cleanup();

        void MakeColor(Utils::Device::QueueType dstQueue = Utils::Device::QueueType::None);

        void MakeDepthStencil(Utils::Device::QueueType dstQueue = Utils::Device::QueueType::None);

        void MakeDepthStencilReadOnly(Utils::Device::QueueType dstQueue = Utils::Device::QueueType::None);

        void MakeStencil(Utils::Device::QueueType dstQueue = Utils::Device::QueueType::None);

        void MakePresent(Utils::Device::QueueType dstQueue = Utils::Device::QueueType::None);

        void MakeTransferSrc(Utils::Device::QueueType dstQueue = Utils::Device::QueueType::None);

        void MakeTransferDst(Utils::Device::QueueType dstQueue = Utils::Device::QueueType::None);

        void MakeShaderReadable(Utils::Device::QueueType dstQueue = Utils::Device::QueueType::None);

        void MoveToLayout(VkImageLayout newLayout, Utils::Device::QueueType dstQueue = Utils::Device::QueueType::None);

        void CopyFromBuffer(VkBuffer buffer, VkExtent3D imageExtent, uint32_t layerCount = 1);

        [[nodiscard]] VkImage GetImage() const
        {
            return m_image;
        }

        [[nodiscard]] VkImageView GetImageView() const
        {
            return m_imageView;
        }

        [[nodiscard]] VkImageLayout GetImageLayout() const
        {
            return m_imageLayout;
        }

        [[nodiscard]] VkExtent3D GetExtent() const
        {
            return m_extent;
        }

        [[nodiscard]] VkFormat GetFormat() const
        {
            return m_format;
        }

        [[nodiscard]] uint32_t GetMipLevels() const
        {
            return m_mipLevels;
        }

        [[nodiscard]] uint32_t GetArrayLayers() const
        {
            return m_arrayLayers;
        }

        [[nodiscard]] VkImageUsageFlags GetUsage() const
        {
            return m_usage;
        }

        [[nodiscard]] VkImageAspectFlags GetAspectMask() const
        {
            return m_aspectMask;
        }

        [[nodiscard]] VkImageType GetImageType() const
        {
            return m_imageType;
        }

    private:
        Device::RenderingDevice &m_device;

        VkImage m_image = VK_NULL_HANDLE;
        VkImageView m_imageView = VK_NULL_HANDLE;
        VkImageLayout m_imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        VmaAllocation m_allocation = VK_NULL_HANDLE;

        uint32_t m_srcFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

        VkFormat m_format = VK_FORMAT_UNDEFINED;
        VkExtent3D m_extent = {};
        uint32_t m_mipLevels = 1;
        uint32_t m_arrayLayers = 1;
        VkImageUsageFlags m_usage = 0;
        VkImageAspectFlags m_aspectMask = 0;
        VkImageType m_imageType = VK_IMAGE_TYPE_2D;
        VkImageViewType m_viewType = VK_IMAGE_VIEW_TYPE_2D;
        VkSampleCountFlagBits m_samples = VK_SAMPLE_COUNT_1_BIT;
        VkImageTiling m_tiling = VK_IMAGE_TILING_OPTIMAL;
        VkImageCreateFlags m_createFlags = 0;
        VkImageLayout m_initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        bool m_isExternal = false;

        bool CreateImage();

        bool CreateImageView();

        void DestroyImage();

        void DestroyImageView();
    };

    using ImageHandle = std::shared_ptr<Image>;
}
