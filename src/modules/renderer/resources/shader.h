//
// Created by lepag on 6/10/2025.
//

#pragma once

#include <optional>
#include <string>
#include <volk.h>
#include "utilities/shader.h"

namespace GyroEngine::Device
{
    class RenderingDevice;
}

namespace GyroEngine::Resources
{

    class Shader {
    public:
        explicit Shader(Device::RenderingDevice& device): m_device(device) {}
        ~Shader() { Cleanup(); }

        Shader& SetShaderPath(const std::string& path);
        Shader& SetShaderStage(Utils::Shader::ShaderStage type);

        bool Init();
        void Cleanup();

        [[nodiscard]] std::optional<Utils::Shader::ShaderReflection> Reflect() const;

        [[nodiscard]] VkShaderModule GetShaderModule() const
        {
            return m_shaderModule;
        }

        [[nodiscard]] std::string GetShaderPath() const
        {
            return m_shaderPath;
        }

        [[nodiscard]] Utils::Shader::ShaderStage GetShaderStage() const
        {
            return m_shaderStage;
        }
    private:
        Device::RenderingDevice& m_device;

        VkShaderModule m_shaderModule = VK_NULL_HANDLE;
        std::string m_shaderPath;
        Utils::Shader::ShaderStage m_shaderStage = Utils::Shader::ShaderStage::Vertex;

        bool CreateShader();
    };

    using ShaderHandle = std::shared_ptr<Shader>;
}
