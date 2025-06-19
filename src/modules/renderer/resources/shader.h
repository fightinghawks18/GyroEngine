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
    private:
        Device::RenderingDevice& m_device;

        VkShaderModule m_shaderModule = VK_NULL_HANDLE;
        std::string m_shaderPath;

        bool CreateShader();
    };
}
