//
// Created by lepag on 6/10/2025.
//

#pragma once

#include <filesystem>
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <string>
#include <volk.h>
#include <shaderc/shaderc.hpp>

namespace shaderutils
{
    enum class ShaderType
    {
        Graphics,
        Compute
    };

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

    inline std::string readShaderSource(const std::string& filePath) {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open shader file: " + filePath);
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        file.close();

        return buffer.str();
    }

    inline std::vector<uint32_t> compileShader(const std::string& source, shaderc_shader_kind kind) {
        std::string shaderSource = readShaderSource(source);

        shaderc::Compiler compiler;
        shaderc::CompileOptions options;

        shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(shaderSource, kind, "shader");
        if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
            throw std::runtime_error("Shader compilation failed: " + result.GetErrorMessage());
        }

        return {result.cbegin(), result.cend()};
    }

    inline void compileShaderToFile(const std::string& sourcePath, shaderc_shader_kind kind) {
        std::string shaderSource = readShaderSource(sourcePath);

        shaderc::Compiler compiler;
        shaderc::CompileOptions options;

        shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(shaderSource, kind, "shader");
        if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
            throw std::runtime_error("Shader compilation failed: " + result.GetErrorMessage());
        }

        std::string outputPath = sourcePath + ".spv";
        std::ofstream outFile(outputPath, std::ios::binary);
        if (!outFile.is_open()) {
            throw std::runtime_error("Failed to create output file: " + outputPath);
        }

        outFile.write(reinterpret_cast<const char*>(result.cbegin()),
                      (result.cend() - result.cbegin()) * sizeof(uint32_t));
        outFile.close();
    }
}