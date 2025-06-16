//
// Created by lepag on 6/10/2025.
//

#pragma once

#include <volk.h>
#include "device_resource.h"

class Sampler final : public IDeviceResource {
public:
    explicit Sampler(RenderingDevice& device): IDeviceResource(device) {}
    ~Sampler() override { Sampler::cleanup(); }

    Sampler& setMinFilter(VkFilter minFilter);
    Sampler& setMagFilter(VkFilter magFilter);
    Sampler& setAddressModeU(VkSamplerAddressMode modeU);
    Sampler& setAddressModeV(VkSamplerAddressMode modeV);
    Sampler& setAddressModeW(VkSamplerAddressMode modeW);
    Sampler& setMipLodBias(float bias);
    Sampler& setMinLod(float minLod);
    Sampler& setMaxLod(float maxLod);
    Sampler& setAnisotropy(bool enable);
    Sampler& setAnisotropyLevel(float level);
    Sampler& setCompareOp(VkCompareOp compareOp);

    bool init() override;
    void cleanup() override;

    [[nodiscard]] VkSampler getSampler() const {
        return m_sampler;
    }
private:
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

    bool createSampler();
};
