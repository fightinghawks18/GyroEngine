//
// Created by lepag on 6/10/2025.
//

#pragma once

#include <string>
#include <volk.h>
#include "utilities/shader.h"
#include "device_resource.h"

class Shader : public IDeviceResource {
public:
    explicit Shader(RenderingDevice& device): IDeviceResource(device) {}
    ~Shader() override { Shader::cleanup(); }

    Shader& setShaderPath(const std::string& path);

    bool init() override;
    void cleanup() override;

    [[nodiscard]] VkShaderModule getShaderModule() const
    {
        return m_shaderModule;
    }
private:
    VkShaderModule m_shaderModule = VK_NULL_HANDLE;
    std::string m_shaderPath;

    bool createShader();
};
