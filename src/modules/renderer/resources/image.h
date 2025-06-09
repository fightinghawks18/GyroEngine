//
// Created by lepag on 6/8/2025.
//

#pragma once

#include <string>
#include <volk.h>
#include "../implementation/vma_implementation.h"

#include "utilities/image.h"
#include "utilities/renderer.h"

class RenderingDevice;

class Image {
public:
    explicit Image(RenderingDevice& image) : m_device(image) {}
    ~Image() = default;

    //static Image fromFile(RenderingDevice& device, const std::string& filePath);

    Image& setFormat(VkFormat format);
    Image& setExtent(VkExtent3D extent);
    Image& setMipLevels(uint32_t mipLevels);
    Image& setArrayLayers(uint32_t arrayLayers);
    Image& setUsage(VkImageUsageFlags usage);
    Image& setAspectMask(VkImageAspectFlags aspectMask);
    Image& setImageType(VkImageType imageType);
    Image& setViewType(VkImageViewType viewType);
    Image& setSamples(VkSampleCountFlagBits samples);
    Image& setTiling(VkImageTiling tiling);
    Image& setCreateFlags(VkImageCreateFlags createFlags);
    Image& setInitialLayout(VkImageLayout initialLayout);

    bool init(VkImage externalImage = VK_NULL_HANDLE, VkImageView imageView = VK_NULL_HANDLE);
    void cleanup();

    void makeColor();
    void makeDepth();
    void makeStencil();
    void makePresent();
    void makeTransferSrc();
    void makeTransferDst();
    void moveToLayout(VkImageLayout newLayout);
    void copyFromBuffer(VkBuffer buffer, uint32_t layerCount = 1);

    [[nodiscard]] VkImage getImage() const {
        return m_image;
    }

    [[nodiscard]] VkImageView getImageView() const {
        return m_imageView;
    }

    [[nodiscard]] VkImageLayout getImageLayout() const {
        return m_imageLayout;
    }
private:
    RenderingDevice& m_device;

    VkImage m_image = VK_NULL_HANDLE;
    VkImageView m_imageView = VK_NULL_HANDLE;
    VkImageLayout m_imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VmaAllocation m_allocation = VK_NULL_HANDLE;

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

    bool createImage();
    bool createImageView();

    void destroyImage();
    void destroyImageView();
};
