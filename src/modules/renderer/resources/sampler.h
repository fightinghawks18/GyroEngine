//
// Created by lepag on 6/10/2025.
//

#pragma once

#include <memory>
#include <volk.h>

namespace GyroEngine::Device
{
    class RenderingDevice;
}

namespace GyroEngine::Resources
{
    class Sampler final  {
    public:
        explicit Sampler(Device::RenderingDevice& device): m_device(device) {}
        ~Sampler() { Cleanup(); }

        Sampler& SetMinFilter(VkFilter minFilter);
        Sampler& SetMagFilter(VkFilter magFilter);
        Sampler& SetAddressModeU(VkSamplerAddressMode modeU);
        Sampler& SetAddressModeV(VkSamplerAddressMode modeV);
        Sampler& SetAddressModeW(VkSamplerAddressMode modeW);
        Sampler& SetMipLodBias(float bias);
        Sampler& SetMinLod(float minLod);
        Sampler& SetMaxLod(float maxLod);
        Sampler& SetAnisotropy(bool enable);
        Sampler& SetAnisotropyLevel(float level);
        Sampler& SetCompareOp(VkCompareOp compareOp);

        bool Init();
        void Cleanup();

        [[nodiscard]] VkSampler GetSampler() const {
            return m_sampler;
        }
    private:
        Device::RenderingDevice& m_device;

        VkSampler m_sampler = VK_NULL_HANDLE;

        VkFilter m_minFilter = VK_FILTER_LINEAR;
        VkFilter m_magFilter = VK_FILTER_LINEAR;
        VkSamplerAddressMode m_addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        VkSamplerAddressMode m_addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        VkSamplerAddressMode m_addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        VkSamplerMipmapMode m_mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        float m_mipLodBias = 0.0f;
        float m_minLod = 0.0f;
        float m_maxLod = VK_LOD_CLAMP_NONE;
        VkBool32 m_anisotropyEnable = VK_FALSE;
        float m_anisotropyLevel = 0.0f;
        VkCompareOp m_compareOp = VK_COMPARE_OP_ALWAYS;

        bool CreateSampler();
    };

    using SamplerHandle = std::shared_ptr<Sampler>;
}