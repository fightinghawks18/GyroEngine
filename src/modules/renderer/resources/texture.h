//
// Created by lepag on 6/19/2025.
//

#pragma once

#include <memory>

#include <glm/glm.hpp>

#include "image.h"
#include "sampler.h"

namespace GyroEngine::Device
{
    class RenderingDevice;
}

namespace GyroEngine::Resources
{
    enum class TextureChannel
    {
        RGB,
        RGBA
    };

    class Texture
    {
    public:
        explicit Texture(Device::RenderingDevice& device)
            : m_device(device), m_texturePathDirty(false) {}
        ~Texture() { Destroy(); }

        Texture& SetTexturePath(const std::string& texturePath)
        {
            m_texturePath = texturePath;
            m_texturePathDirty = true;
            return *this;
        }

        bool Init();
        bool Generate();
        void Destroy();

        void Update();

        [[nodiscard]] glm::vec3& GetSize()
        {
            return m_size;
        }

        [[nodiscard]] SamplerHandle GetSampler()
        {
            return m_sampler;
        }

        [[nodiscard]] ImageHandle GetImage() const
        {
            return m_image;
        }

        [[nodiscard]] TextureChannel GetChannel() const
        {
            return m_channel;
        }

        [[nodiscard]] const std::string& GetTexturePath() const
        {
            return m_texturePath;
        }
    private:
        Device::RenderingDevice& m_device;

        glm::vec3 m_size{ 800.0f };

        SamplerHandle m_sampler = nullptr;
        ImageHandle m_image = nullptr;
        TextureChannel m_channel = TextureChannel::RGBA;
        std::string m_texturePath;
        bool m_texturePathDirty;

        bool CreateSampler();
        void DestroySampler();

        bool CreateImage();
        void DestroyImage();

        [[nodiscard]] Utils::Image::ImageData* LoadTextureFromFile() const;
        bool CopyTextureToImage(const Utils::Image::ImageData* imageData);
    };

    using TextureHandle = std::shared_ptr<Texture>;

}
