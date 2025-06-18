//
// Created by lepag on 6/8/2025.
//

#include "image.h"

#include "context/rendering_device.h"
#include "utilities/renderer.h"

//Image Image::fromFile(RenderingDevice &device, const std::string &filePath) {}

namespace GyroEngine::Resources
{
    Image &Image::SetFormat(VkFormat format)
{
    m_format = format;
    return *this;
}

Image &Image::SetExtent(VkExtent3D extent)
{
    m_extent = extent;
    return *this;
}

Image &Image::SetMipLevels(const uint32_t mipLevels)
{
    m_mipLevels = mipLevels;
    return *this;
}

Image &Image::SetArrayLayers(const uint32_t arrayLayers)
{
    m_arrayLayers = arrayLayers;
    return *this;
}

Image &Image::SetUsage(VkImageUsageFlags usage)
{
    m_usage = usage;
    return *this;
}

Image &Image::SetAspectMask(VkImageAspectFlags aspectMask)
{
    m_aspectMask = aspectMask;
    return *this;
}

Image &Image::SetImageType(VkImageType imageType)
{
    m_imageType = imageType;
    return *this;
}

Image &Image::SetViewType(VkImageViewType viewType)
{
    m_viewType = viewType;
    return *this;
}

Image &Image::SetSamples(VkSampleCountFlagBits samples)
{
    m_samples = samples;
    return *this;
}

Image &Image::SetTiling(VkImageTiling tiling)
{
    m_tiling = tiling;
    return *this;
}

Image &Image::SetCreateFlags(VkImageCreateFlags createFlags)
{
    m_createFlags = createFlags;
    return *this;
}

Image &Image::SetInitialLayout(VkImageLayout initialLayout)
{
    m_initialLayout = initialLayout;
    return *this;
}

bool Image::Init(VkImage externalImage, VkImageView imageView)
{
    if (externalImage != VK_NULL_HANDLE && imageView != VK_NULL_HANDLE)
    {
        m_image = externalImage;
        m_imageView = imageView;
        m_isExternal = true;
        return true;
    }
    if (!CreateImage())
    {
        return false;
    }
    if (!CreateImageView())
    {
        DestroyImage();
        return false;
    }
    return true;
}

void Image::Cleanup()
{
    DestroyImage();
    DestroyImageView();
}

void Image::MakeColor(const Utils::Device::QueueType dstQueue)
{
    MoveToLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, dstQueue);
}

void Image::MakeDepth(const Utils::Device::QueueType dstQueue)
{
    MoveToLayout(VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, dstQueue);
}

void Image::MakeStencil(const Utils::Device::QueueType dstQueue)
{
    MoveToLayout(VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL, dstQueue);
}

void Image::MakePresent(const Utils::Device::QueueType dstQueue)
{
    MoveToLayout(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, dstQueue);
}

void Image::MakeTransferSrc(const Utils::Device::QueueType dstQueue)
{
    MoveToLayout(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstQueue);
}

void Image::MakeTransferDst(const Utils::Device::QueueType dstQueue)
{
    MoveToLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, dstQueue);
}

void Image::MakeShaderReadable(const Utils::Device::QueueType dstQueue)
{
    MoveToLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, dstQueue);
}

void Image::MoveToLayout(VkImageLayout newLayout, const Utils::Device::QueueType dstQueue)
{
    uint32_t dstQueueIndex = 0;
    if (dstQueue == Utils::Device::QueueType::None)
    {
        dstQueueIndex = VK_QUEUE_FAMILY_IGNORED;
    } else
    {
        Device::DeviceQueue queue = m_device.GetDeviceFamilies().GetQueue(dstQueue);
        if (queue.isValid())
        {
            dstQueueIndex = queue.family;
        } else
        {
            dstQueueIndex = VK_QUEUE_FAMILY_IGNORED;
        }
    }

    Utils::Renderer::SubmitOneTimeCommand(
        m_device.GetLogicalDevice(),
        m_device.GetCommandPool(),
        m_device.GetDeviceFamilies().GetGraphicsQueue().queue,
        [&](VkCommandBuffer commandBuffer)
        {
            VkImageMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.image = m_image;
            barrier.oldLayout = m_imageLayout;
            barrier.newLayout = newLayout;
            barrier.srcQueueFamilyIndex = m_srcFamilyIndex;
            barrier.dstQueueFamilyIndex = dstQueueIndex;
            barrier.subresourceRange.aspectMask = m_aspectMask;
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = m_mipLevels;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = m_arrayLayers;

            VkPipelineStageFlags srcStageMask = Utils::Image::GetSourceStageFlags(m_imageLayout);
            VkPipelineStageFlags dstStageMask = Utils::Image::GetDestinationStageFlags(newLayout);

            vkCmdPipelineBarrier(
                commandBuffer,
                srcStageMask, dstStageMask,
                0, 0, nullptr,
                0, nullptr,
                1, &barrier
            );

            m_imageLayout = newLayout;
        }
    );
}

void Image::CopyFromBuffer(VkBuffer buffer, const uint32_t layerCount)
{
    VkImageLayout oldLayout = m_imageLayout;
    Utils::Renderer::SubmitOneTimeCommand(
        m_device.GetLogicalDevice(),
        m_device.GetCommandPool(),
        m_device.GetDeviceFamilies().GetGraphicsQueue().queue,
        [&](VkCommandBuffer commandBuffer)
        {
            MakeTransferDst(); // Must be in transfer destination layout before copying
            VkBufferImageCopy region{};
            region.bufferOffset = 0;
            region.bufferRowLength = 0;
            region.bufferImageHeight = 0;
            region.imageSubresource.aspectMask = m_aspectMask;
            region.imageSubresource.mipLevel = 0;
            region.imageSubresource.baseArrayLayer = 0;
            region.imageSubresource.layerCount = layerCount;
            region.imageOffset = {0, 0, 0};
            region.imageExtent = m_extent;

            vkCmdCopyBufferToImage(
                commandBuffer,
                buffer,
                m_image,
                m_imageLayout, // Should be VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
                1,
                &region
            );
        }
    );
    MoveToLayout(oldLayout);
}

bool Image::CreateImage()
{
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = m_imageType;
    imageInfo.extent = m_extent;
    imageInfo.mipLevels = m_mipLevels;
    imageInfo.arrayLayers = m_arrayLayers;
    imageInfo.format = m_format;
    imageInfo.tiling = m_tiling;
    imageInfo.initialLayout = m_initialLayout;
    imageInfo.usage = m_usage;
    imageInfo.samples = m_samples;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.flags = m_createFlags;

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO;

    if (vmaCreateImage(m_device.GetAllocator(), &imageInfo, &allocInfo, &m_image, &m_allocation, nullptr) != VK_SUCCESS)
    {
        return false;
    }
    return true;
}

bool Image::CreateImageView()
{
    VkImageViewCreateInfo imageViewInfo{};
    imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewInfo.image = m_image;
    imageViewInfo.viewType = m_viewType;
    imageViewInfo.format = m_format;
    imageViewInfo.subresourceRange.aspectMask = m_aspectMask;
    imageViewInfo.subresourceRange.baseMipLevel = 0;
    imageViewInfo.subresourceRange.levelCount = m_mipLevels;
    imageViewInfo.subresourceRange.baseArrayLayer = 0;
    imageViewInfo.subresourceRange.layerCount = m_arrayLayers;

    if (vkCreateImageView(m_device.GetLogicalDevice(), &imageViewInfo, nullptr, &m_imageView) != VK_SUCCESS)
    {
        return false;
    }
    return true;
}

void Image::DestroyImage()
{
    if (m_isExternal) { return; }
    if (m_image != VK_NULL_HANDLE)
    {
        vmaDestroyImage(m_device.GetAllocator(), m_image, m_allocation);
        m_image = VK_NULL_HANDLE;
        m_allocation = VK_NULL_HANDLE;
    }
}

void Image::DestroyImageView()
{
    if (m_imageView != VK_NULL_HANDLE)
    {
        vkDestroyImageView(m_device.GetLogicalDevice(), m_imageView, nullptr);
        m_imageView = VK_NULL_HANDLE;
    }
}
}
