//
// Created by lepag on 6/21/2025.
//

#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <volk.h>

#include <spirv-reflect/spirv.h>

#include "buffer.h"
#include "image.h"
#include "sampler.h"
#include "shader.h"

namespace GyroEngine::Rendering
{
    struct FrameContext;
}

namespace GyroEngine::Device
{
    class RenderingDevice;
}

namespace GyroEngine::Resources
{
    class PipelineBindings
    {
        struct Binding
        {
            std::string name;
            uint32_t binding = 0;
            VkDescriptorType type = VK_DESCRIPTOR_TYPE_MAX_ENUM;
            VkShaderStageFlags stageFlags = VK_SHADER_STAGE_ALL;

            VkDescriptorSetLayoutBinding layoutBinding = {};
        };

        struct AllocatedSet
        {
            VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
            VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
        };

        struct Set
        {
            uint32_t set = 0;
            VkDescriptorType type = VK_DESCRIPTOR_TYPE_MAX_ENUM;

            VkDescriptorSetLayout layout = VK_NULL_HANDLE;
            std::vector<Binding> bindings;
            std::vector<AllocatedSet> descriptorSets;
        };

        struct PushConstantMember
        {
            std::string name;
            uint32_t offset = 0;
            uint32_t size = 0;
            void* data;
        };

        struct PushConstantBlock
        {
            std::string name;
            uint32_t offset = 0;
            uint32_t size = 0;
            VkShaderStageFlags stageFlags = VK_SHADER_STAGE_ALL;

            std::vector<PushConstantMember> members;
        };
    public:
        struct VertexInput
        {
            std::string name;
            uint32_t location = 0;
            VkFormat format = VK_FORMAT_UNDEFINED;
            uint32_t offset = 0;
            VkVertexInputRate inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        };

        explicit PipelineBindings(Device::RenderingDevice& device) : m_device(device) {}
        ~PipelineBindings() { Cleanup(); }

        PipelineBindings& AddShader(const ShaderHandle& shader)
        {
            m_shaderStages.push_back(shader);
            return *this;
        }

        bool Init();
        void Cleanup();

        void UpdateDescriptorBuffer(const std::string& name, const BufferHandle& buffer, uint32_t index);
        void UpdateDescriptorImage(const std::string& name, const SamplerHandle& sampler, const ImageHandle& image, uint32_t index);
        void UpdatePushConstant(const std::string& block, const std::string& name, const void* data, size_t size, uint32_t offset = 0);

        void BindConstants(const Rendering::FrameContext& frameContext, VkPipelineLayout pipelineLayout);
        void Bind(const Rendering::FrameContext& frameContext, VkPipelineLayout pipelineLayout);

        bool DoesBindingExist(const std::string& name) { return GetBinding(name).has_value(); }
        bool DoesPushConstantExist(const std::string& block, const std::string& name) { return GetPushConstant(block, name).has_value(); }

        [[nodiscard]] std::vector<ShaderHandle>& GetShaderStages()
        {
            return m_shaderStages;
        }

        [[nodiscard]] std::vector<PushConstantBlock>& GetPushConstants()
        {
            return m_pushConstants;
        }

        [[nodiscard]] std::vector<VkDescriptorPool>& GetDescriptorPools()
        {
            return m_descriptorPools;
        }

        [[nodiscard]] std::vector<VertexInput>& GetVertexInputs()
        {
            return m_vertexInputs;
        }

        [[nodiscard]] std::vector<std::shared_ptr<Set>> GetSets()
        {
            return m_sets;
        }
    private:
        Device::RenderingDevice& m_device;

        std::vector<VkDescriptorPool> m_descriptorPools;
        std::vector<ShaderHandle> m_shaderStages;
        std::vector<PushConstantBlock> m_pushConstants;
        std::vector<VertexInput> m_vertexInputs;
        std::vector<std::shared_ptr<Set>> m_sets;
        std::unordered_map<ShaderHandle, SpvReflectShaderModule> m_spvModules;

        std::optional<std::pair<Set, Binding>> GetBinding(const std::string& name);
        std::optional<std::pair<PushConstantBlock, PushConstantMember>> GetPushConstant(const std::string& block, const std::string& name);

        bool CreateSpvModules();
        void DestroySpvModules();

        bool GetInputAttributes();

        bool CreateDescriptorSetLayouts();
        void DestroyDescriptorSetLayouts();

        bool CreatePushConstantRanges();
        void DestroyPushConstantRanges();

        bool CreateDescriptorPool();
        void DestroyDescriptorPool();

        [[nodiscard]] bool AllocateDescriptorSets() const;
        void DestroyDescriptorSets() const;

        static VkDescriptorType ToVkDescriptorType(const SpvReflectDescriptorType reflectType) {
            switch (reflectType) {
            case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER:
                return VK_DESCRIPTOR_TYPE_SAMPLER;
            case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
                return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
                return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
            case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE:
                return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
                return VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
            case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
                return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
            case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
                return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER:
                return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
                return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
            case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
                return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
            case SPV_REFLECT_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
                return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
            default:
                return VK_DESCRIPTOR_TYPE_MAX_ENUM;
            }
        }
    };
} // Resources
// GyroEngine
