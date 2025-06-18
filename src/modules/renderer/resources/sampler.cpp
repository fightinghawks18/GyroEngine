//
// Created by lepag on 6/10/2025.
//

#include "sampler.h"
#include "context/rendering_device.h"

namespace GyroEngine::Resources
{

Sampler& Sampler::SetMinFilter(VkFilter minFilter)
{
    m_minFilter = minFilter;
    return *this;
}

Sampler& Sampler::SetMagFilter(VkFilter magFilter)
{
    m_magFilter = magFilter;
    return *this;
}

Sampler& Sampler::SetAddressModeU(VkSamplerAddressMode modeU)
{
    m_addressModeU = modeU;
    return *this;
}

Sampler& Sampler::SetAddressModeV(VkSamplerAddressMode modeV)
{
    m_addressModeV = modeV;
    return *this;
}

Sampler& Sampler::SetAddressModeW(VkSamplerAddressMode modeW)
{
    m_addressModeW = modeW;
    return *this;
}

Sampler& Sampler::SetMipLodBias(const float bias)
{
    m_mipLodBias = bias;
    return *this;
}

Sampler& Sampler::SetMinLod(const float minLod)
{
    m_minLod = minLod;
    return *this;
}

Sampler& Sampler::SetMaxLod(const float maxLod)
{
    m_maxLod = maxLod;
    return *this;
}

Sampler& Sampler::SetAnisotropy(const bool enable)
{
    m_anisotropyEnable = enable;
    return *this;
}

Sampler& Sampler::SetAnisotropyLevel(const float level)
{
    m_anisotropyLevel = level;
    return *this;
}

Sampler& Sampler::SetCompareOp(VkCompareOp compareOp)
{
    m_compareOp = compareOp;
    return *this;
}

bool Sampler::Init()
{
    if (m_sampler != VK_NULL_HANDLE)
    {
        return false;
    }
    return CreateSampler();
}

void Sampler::Cleanup()
{
    if (m_sampler != VK_NULL_HANDLE)
    {
        vkDestroySampler(m_device.GetLogicalDevice(), m_sampler, nullptr);
        m_sampler = VK_NULL_HANDLE;
    }
}

bool Sampler::CreateSampler()
{
    if (m_anisotropyEnable && m_anisotropyLevel > m_device.GetPhysicalDeviceProperties().limits.maxSamplerAnisotropy) {
        Logger::LogError("Anisotropy level exceeds device limits");
        return false;
    }

    VkSamplerCreateInfo samplerInfo = {};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

    samplerInfo.magFilter = m_magFilter;
    samplerInfo.minFilter = m_minFilter;

    samplerInfo.addressModeU = m_addressModeU;
    samplerInfo.addressModeV = m_addressModeV;
    samplerInfo.addressModeW = m_addressModeW;

    samplerInfo.mipmapMode = m_mipmapMode;

    samplerInfo.mipLodBias = m_mipLodBias;
    samplerInfo.minLod = m_minLod;
    samplerInfo.maxLod = m_maxLod;

    samplerInfo.anisotropyEnable = m_anisotropyEnable;
    samplerInfo.maxAnisotropy = m_anisotropyLevel;

    samplerInfo.compareOp = m_compareOp;

    VkResult result = vkCreateSampler(m_device.GetLogicalDevice(), &samplerInfo, nullptr, &m_sampler);
    if (result != VK_SUCCESS)
    {
        Logger::LogError("Failed to create sampler");
        return false;
    }
    return true;
}
}
