//
// Created by lepag on 6/10/2025.
//

#include "shader.h"

#include "context/rendering_device.h"

Shader& Shader::setShaderPath(const std::string& path)
{
    m_shaderPath = path;
    return *this;
}

bool Shader::init()
{
    if (m_shaderModule != VK_NULL_HANDLE)
    {
        return false;
    }
    return createShader();
}

void Shader::cleanup()
{
    if (m_shaderModule != VK_NULL_HANDLE)
    {
        vkDestroyShaderModule(m_device.getLogicalDevice(), m_shaderModule, nullptr);
        m_shaderModule = VK_NULL_HANDLE;
    }
}

bool Shader::createShader()
{
    std::vector<char> shaderSource = shaderutils::readShaderSPV(m_shaderPath.c_str());
    if (shaderSource.empty())
    {
        Printer::error("Failed to read shader code: " + m_shaderPath);
        return false;
    }

    if (shaderSource.size() % 4 != 0)
    {
        Printer::error("Shader code size is not a multiple of 4");
        return false;
    }

    VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
    shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderModuleCreateInfo.codeSize = shaderSource.size();
    shaderModuleCreateInfo.pCode = reinterpret_cast<uint32_t*>(shaderSource.data());

    VkResult result = vkCreateShaderModule(m_device.getLogicalDevice(), &shaderModuleCreateInfo, nullptr, &m_shaderModule);
    if (result != VK_SUCCESS)
    {
        Printer::error("Failed to create shader module: " + m_shaderPath);
        return false;
    }
    return true;
}
