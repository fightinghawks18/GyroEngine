//
// Created by lepag on 6/10/2025.
//

#include "sampler.h"
#include "context/rendering_device.h"

Sampler& Sampler::setMinFilter(VkFilter minFilter)
{
    m_minFilter = minFilter;
    return *this;
}

Sampler& Sampler::setMagFilter(VkFilter magFilter)
{
    m_magFilter = magFilter;
    return *this;
}

Sampler& Sampler::setAddressModeU(VkSamplerAddressMode modeU)
{
    m_addressModeU = modeU;
    return *this;
}

Sampler& Sampler::setAddressModeV(VkSamplerAddressMode modeV)
{
    m_addressModeV = modeV;
    return *this;
}

Sampler& Sampler::setAddressModeW(VkSamplerAddressMode modeW)
{
    m_addressModeW = modeW;
    return *this;
}

Sampler& Sampler::setMipLodBias(float bias)
{
    m_mipLodBias = bias;
    return *this;
}

Sampler& Sampler::setMinLod(float minLod)
{
    m_minLod = minLod;
    return *this;
}

Sampler& Sampler::setMaxLod(float maxLod)
{
    m_maxLod = maxLod;
    return *this;
}

Sampler& Sampler::setAnisotropy(bool enable)
{
    m_anisotropyEnable = enable;
    return *this;
}

Sampler& Sampler::setAnisotropyLevel(float level)
{
    m_anisotropyLevel = level;
    return *this;
}

Sampler& Sampler::setCompareOp(VkCompareOp compareOp)
{
    m_compareOp = compareOp;
    return *this;
}

bool Sampler::init()
{
    if (m_sampler != VK_NULL_HANDLE)
    {
        return false;
    }
    return createSampler();
}

void Sampler::cleanup()
{
    if (m_sampler != VK_NULL_HANDLE)
    {
        vkDestroySampler(m_device.getLogicalDevice(), m_sampler, nullptr);
        m_sampler = VK_NULL_HANDLE;
    }
}

bool Sampler::createSampler()
{
    if (m_anisotropyEnable && m_anisotropyLevel > m_device.getPhysicalDeviceProperties().limits.maxSamplerAnisotropy) {
        Printer::error("Anisotropy level exceeds device limits");
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

    VkResult result = vkCreateSampler(m_device.getLogicalDevice(), &samplerInfo, nullptr, &m_sampler);
    if (result != VK_SUCCESS)
    {
        Printer::error("Failed to create sampler");
        return false;
    }
    return true;
}
