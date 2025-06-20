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
#include <spirv-reflect/spirv_reflect.h>

namespace GyroEngine::Utils::Shader
{
    enum class ShaderType
    {
        Graphics,
        Compute
    };

    struct ShaderBinding {
        std::string name;
        uint32_t set;
        uint32_t binding;
        VkDescriptorType type;
        VkShaderStageFlags stageFlags;
        uint32_t count;

        bool operator==(const ShaderBinding& other) const {
            return binding == other.binding &&
                   type == other.type &&
                   set == other.set &&
                   count == other.count &&
                   stageFlags == other.stageFlags;
        }
    };

    struct ShaderInputAttribute {
        std::string name;
        uint32_t location;
        VkFormat format;
    };

    struct ShaderPushConstantMember
    {
        std::string name;
        uint32_t offset;
        uint32_t size;
    };

    struct ShaderPushConstant
    {
        std::string name;
        uint32_t size;
        uint32_t offset;
        VkShaderStageFlags stage;
        std::vector<ShaderPushConstantMember> members;
    };

    struct ShaderReflection
    {
        std::vector<ShaderBinding> descriptorSets;
        std::vector<ShaderPushConstant> pushConstants;
        std::vector<ShaderInputAttribute> inputAttributes;
    };

    struct ShaderReflectionGroup
    {
        std::vector<ShaderReflection> reflections;

        ShaderReflection CreateReflectionFromGroup() const
        {
            ShaderReflection merged;

            // Merge descriptor sets
            for (const auto& refl : reflections) {
                for (const auto& binding : refl.descriptorSets) {
                    auto it = std::find_if(
                        merged.descriptorSets.begin(), merged.descriptorSets.end(),
                        [&](const ShaderBinding& b) {
                            return b.set == binding.set && b.binding == binding.binding;
                        });
                    if (it != merged.descriptorSets.end()) {
                        it->stageFlags |= binding.stageFlags;
                    } else {
                        merged.descriptorSets.push_back(binding);
                    }
                }
            }

            // Merge push constants (by offset/size)
            for (const auto& refl : reflections) {
                for (const auto& pc : refl.pushConstants) {
                    auto it = std::find_if(
                        merged.pushConstants.begin(), merged.pushConstants.end(),
                        [&](const ShaderPushConstant& mpc) {
                            return mpc.offset == pc.offset && mpc.size == pc.size;
                        });
                    if (it != merged.pushConstants.end()) {
                        it->stage |= pc.stage;
                    } else {
                        merged.pushConstants.push_back(pc);
                    }
                }
            }

            // Merge input attributes (by location)
            for (const auto& refl : reflections) {
                for (const auto& attr : refl.inputAttributes) {
                    auto it = std::find_if(
                        merged.inputAttributes.begin(), merged.inputAttributes.end(),
                        [&](const ShaderInputAttribute& a) {
                            return a.location == attr.location;
                        });
                    if (it == merged.inputAttributes.end()) {
                        merged.inputAttributes.push_back(attr);
                    }
                }
            }

            return merged;
        }
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

    /// @brief Converts a SPIR-V shader file into reflection data.
    inline ShaderReflection ReflectShaderSPV(const std::string& filePath)
    {
        ShaderReflection reflection;
        SpvReflectShaderModule reflect;

        // Read SPIR-V binary data
        const std::vector<char> spvData = ReadShaderSPV(filePath);
        if (spvData.empty()) {
            throw std::runtime_error("SPIR-V data is empty for file: " + filePath);
        }

        // Create SPIR-V reflection module
        SpvReflectResult result = spvReflectCreateShaderModule(spvData.size(), spvData.data(), &reflect);
        if (result != SPV_REFLECT_RESULT_SUCCESS) {
            throw std::runtime_error("Failed to create SPIR-V reflection module for file: " + filePath);
        }

        // Reflect descriptor sets
        uint32_t setCount = 0;
        result = spvReflectEnumerateDescriptorSets(&reflect, &setCount, nullptr);
        if (result != SPV_REFLECT_RESULT_SUCCESS) {
            spvReflectDestroyShaderModule(&reflect);
            throw std::runtime_error("Failed to enumerate descriptor sets for file: " + filePath);
        }

        std::vector<SpvReflectDescriptorSet*> sets(setCount);
        spvReflectEnumerateDescriptorSets(&reflect, &setCount, sets.data());

        for (const auto* set : sets) {
            for (uint32_t i = 0; i < set->binding_count; ++i) {
                const SpvReflectDescriptorBinding* binding = set->bindings[i];
                ShaderBinding shaderBinding;
                shaderBinding.name = binding->name ? binding->name : "";
                shaderBinding.set = binding->set;
                shaderBinding.binding = binding->binding;
                shaderBinding.type = static_cast<VkDescriptorType>(binding->descriptor_type);
                shaderBinding.count = binding->count;
                shaderBinding.stageFlags = reflect.shader_stage;
                reflection.descriptorSets.push_back(shaderBinding);
            }
        }

        // Reflect push constants
        uint32_t pushConstantCount = 0;
        result = spvReflectEnumeratePushConstantBlocks(&reflect, &pushConstantCount, nullptr);
        if (result != SPV_REFLECT_RESULT_SUCCESS) {
            spvReflectDestroyShaderModule(&reflect);
            throw std::runtime_error("Failed to enumerate push constant blocks for file: " + filePath);
        }

        std::vector<SpvReflectBlockVariable*> pushConstants(pushConstantCount);
        spvReflectEnumeratePushConstantBlocks(&reflect, &pushConstantCount, pushConstants.data());

        for (const auto* pc : pushConstants) {
            ShaderPushConstant shaderPC;
            shaderPC.name = pc->name ? pc->name : "";
            shaderPC.size = pc->size;
            shaderPC.offset = pc->offset;
            shaderPC.stage = reflect.shader_stage;

            // Reflect members of the push constant block
            for (uint32_t i = 0; i < pc->member_count; ++i) {
                const SpvReflectBlockVariable* member = &pc->members[i];
                ShaderPushConstantMember pcMember;
                pcMember.name = member->name ? member->name : "";
                pcMember.offset = member->offset;
                pcMember.size = member->size;
                shaderPC.members.push_back(pcMember);
            }

            reflection.pushConstants.push_back(shaderPC);
        }

        // Reflect input attributes
        uint32_t inputCount = 0;
        result = spvReflectEnumerateInputVariables(&reflect, &inputCount, nullptr);
        if (result != SPV_REFLECT_RESULT_SUCCESS) {
            spvReflectDestroyShaderModule(&reflect);
            throw std::runtime_error("Failed to enumerate input variables for file: " + filePath);
        }

        std::vector<SpvReflectInterfaceVariable*> inputs(inputCount);
        spvReflectEnumerateInputVariables(&reflect, &inputCount, inputs.data());

        for (const auto* input : inputs)
        {
            if ((input->decoration_flags & SPV_REFLECT_DECORATION_BUILT_IN) != 0) continue; // Skip built-in variables (gl_* variables)
            ShaderInputAttribute attribute;
            attribute.name = input->name ? input->name : "";
            attribute.format = static_cast<VkFormat>(input->format);
            attribute.location = input->location;
            reflection.inputAttributes.push_back(attribute);
        }

        // Destroy reflection module and return reflection data
        spvReflectDestroyShaderModule(&reflect);
        return reflection;
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