//
// Created by lepag on 6/10/2025.
//

#pragma once

#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <volk.h>
#include <shaderc/shaderc.hpp>

namespace shaderutils
{
    inline std::vector<char> readShaderSPV(const std::string& filePath) {
        std::ifstream file(filePath, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open shader file: " + filePath);
        }

        size_t fileSize = static_cast<size_t>(file.tellg());
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();

        return buffer;
    }

    inline std::vector<uint32_t> compileShader(const std::string& source, shaderc_shader_kind kind) {
        shaderc::Compiler compiler;
        shaderc::CompileOptions options;

        shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(source, kind, "shader");
        if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
            throw std::runtime_error("Shader compilation failed: " + result.GetErrorMessage());
        }

        return {result.cbegin(), result.cend()};
    }
}