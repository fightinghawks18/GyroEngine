//
// Created by lepag on 6/10/2025.
//

#include "shader.h"

#include "context/rendering_device.h"

namespace GyroEngine::Resources
{
    Shader& Shader::SetShaderPath(const std::string& path)
    {
        m_shaderPath = path;
        return *this;
    }

    bool Shader::Init()
    {
        if (m_shaderModule != VK_NULL_HANDLE)
        {
            return false;
        }
        return CreateShader();
    }

    void Shader::Cleanup()
    {
        if (m_shaderModule != VK_NULL_HANDLE)
        {
            vkDestroyShaderModule(m_device.GetLogicalDevice(), m_shaderModule, nullptr);
            m_shaderModule = VK_NULL_HANDLE;
        }
    }

    bool Shader::CreateShader()
    {
        std::vector<char> shaderSource = Utils::Shader::ReadShaderSPV(m_shaderPath);
        if (shaderSource.empty())
        {
            Printer::LogError("Failed to read shader code: " + m_shaderPath);
            return false;
        }

        if (shaderSource.size() % 4 != 0)
        {
            Printer::LogError("Shader code size is not a multiple of 4");
            return false;
        }

        VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
        shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        shaderModuleCreateInfo.codeSize = shaderSource.size();
        shaderModuleCreateInfo.pCode = reinterpret_cast<uint32_t*>(shaderSource.data());

        VkResult result = vkCreateShaderModule(m_device.GetLogicalDevice(), &shaderModuleCreateInfo, nullptr, &m_shaderModule);
        if (result != VK_SUCCESS)
        {
            Printer::LogError("Failed to create shader module: " + m_shaderPath);
            return false;
        }
        return true;
    }
}
