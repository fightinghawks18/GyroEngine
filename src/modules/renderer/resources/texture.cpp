//
// Created by lepag on 6/19/2025.
//

#include "texture.h"

#include "buffer.h"
#include "context/rendering_device.h"

namespace GyroEngine::Resources
{
    bool Texture::Init()
    {
        if (!CreateSampler())
        {
            Logger::LogError("Failed to create texture sampler");
            return false;
        }
        if (!CreateImage())
        {
            Logger::LogError("Failed to create texture image");
            return false;
        }
        return true;
    }

    bool Texture::Generate()
    {
        auto imageData = LoadTextureFromFile();
        if (!imageData)
        {
            Logger::LogError("Failed to load texture from file");
            return false;
        }
        if (!CopyTextureToImage(imageData))
        {
            Logger::LogError("Failed to copy texture to image");
            return false;
        }
        return true;
    }

    void Texture::Destroy()
    {
        DestroyImage();
        DestroySampler();
    }

    void Texture::Update()
    {

        if (m_texturePathDirty || m_image->GetExtent().width != static_cast<uint32_t>(m_size.x) ||
            m_image->GetExtent().height != static_cast<uint32_t>(m_size.y))
        {
            if (!Generate())
            {
                Logger::LogError("Failed to update texture with path: " + m_texturePath);
                return;
            }
            m_texturePathDirty = false;
        }
    }

    bool Texture::CreateSampler()
    {
        m_sampler = std::make_shared<Sampler>(m_device);
        m_sampler->SetAddressModeU(VK_SAMPLER_ADDRESS_MODE_REPEAT)
                .SetAddressModeV(VK_SAMPLER_ADDRESS_MODE_REPEAT)
                .SetAddressModeW(VK_SAMPLER_ADDRESS_MODE_REPEAT)
                .SetMagFilter(VK_FILTER_LINEAR)
                .SetMinFilter(VK_FILTER_LINEAR)
                .SetAnisotropy(false)
                .SetCompareOp(VK_COMPARE_OP_ALWAYS);
        if (!m_sampler->Init())
        {
            Logger::LogError("Failed to initialize texture sampler");
            return false;
        }
        return true;
    }

    void Texture::DestroySampler()
    {
        if (m_sampler)
        {
            m_sampler.reset();
        }
    }

    bool Texture::CreateImage()
    {
        m_image = std::make_shared<Image>(m_device);
        m_image->SetFormat(VK_FORMAT_R8G8B8A8_UNORM)
                .SetExtent({static_cast<uint32_t>(m_size.x), static_cast<uint32_t>(m_size.y), 1})
                .SetUsage(VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT)
                .SetAspectMask(VK_IMAGE_ASPECT_COLOR_BIT)
                .SetImageType(VK_IMAGE_TYPE_2D)
                .SetViewType(VK_IMAGE_VIEW_TYPE_2D)
                .SetSamples(VK_SAMPLE_COUNT_1_BIT)
                .SetTiling(VK_IMAGE_TILING_OPTIMAL)
                .SetCreateFlags(0)
                .SetInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED);

        if (!m_image->Init())
        {
            Logger::LogError("Failed to initialize texture image");
            return false;
        }
        m_image->MakeShaderReadable();
        return true;
    }

    void Texture::DestroyImage()
    {
        if (m_image)
        {
            m_image.reset();
        }
    }

    Utils::Image::ImageData *Texture::LoadTextureFromFile() const
    {
        const auto imageData = Utils::Image::LoadImageData(m_texturePath, static_cast<int>(m_size.x), static_cast<int>(m_size.y));
        if (!imageData)
        {
            Logger::LogError("Failed to load texture data from path: " + m_texturePath);
            return nullptr;
        }
        return imageData;
    }

    bool Texture::CopyTextureToImage(const Utils::Image::ImageData *imageData)
    {
        auto buffer = std::make_shared<Buffer>(m_device);
        buffer->SetSize(imageData->width * imageData->height * 4)
              .SetUsage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
              .SetMemoryUsage(VMA_MEMORY_USAGE_AUTO_PREFER_HOST)
              .SetSharingMode(VK_SHARING_MODE_EXCLUSIVE);
        if (!buffer->Init())
        {
            Logger::LogError("Failed to create buffer for texture data");
            return false;
        }
        buffer->Map(imageData->data);
        m_image->CopyFromBuffer(buffer->GetBuffer(),
                                {static_cast<uint32_t>(imageData->width), static_cast<uint32_t>(imageData->height), 1},
                                1);

        m_image->MakeShaderReadable();

        buffer.reset();
        delete imageData;
        return true;
    }
}
