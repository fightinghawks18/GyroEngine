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
#include <shaderc/shaderc.hpp>

namespace GyroEngine::Utils::Shader
{
    enum class ShaderType
    {
        Graphics,
        Compute
    };

    inline std::vector<char> ReadShaderSPV(const std::string& filePath) {
        std::ifstream file(filePath, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open shader file: " + filePath);
        }

        const size_t fileSize = file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();

        return buffer;
    }

    inline std::string ReadShaderSource(const std::string& filePath) {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open shader file: " + filePath);
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        file.close();

        return buffer.str();
    }

    inline std::vector<uint32_t> CompileShader(const std::string& source, const shaderc_shader_kind kind) {
        const std::string shaderSource = ReadShaderSource(source);

        const shaderc::Compiler compiler;
        shaderc::CompileOptions options;

        const shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(shaderSource, kind, "shader");
        if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
            throw std::runtime_error("Shader compilation failed: " + result.GetErrorMessage());
        }

        return {result.cbegin(), result.cend()};
    }

    inline void CompileShaderToFile(const std::string& sourcePath, const shaderc_shader_kind kind) {
        const std::string shaderSource = ReadShaderSource(sourcePath);

        const shaderc::Compiler compiler;
        shaderc::CompileOptions options;

        const shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(shaderSource, kind, "shader");
        if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
            throw std::runtime_error("Shader compilation failed: " + result.GetErrorMessage());
        }

        const std::string outputPath = sourcePath + ".spv";
        std::ofstream outFile(outputPath, std::ios::binary);
        if (!outFile.is_open()) {
            throw std::runtime_error("Failed to create output file: " + outputPath);
        }

        outFile.write(reinterpret_cast<const char*>(result.cbegin()),
                      (result.cend() - result.cbegin()) * sizeof(uint32_t));
        outFile.close();
    }
}