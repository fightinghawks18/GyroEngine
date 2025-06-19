//
// Created by lepag on 6/19/2025.
//

#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include <utility>
#include <volk.h>

#include "buffer.h"
#include "image.h"
#include "sampler.h"
#include "utilities/shader.h"

namespace GyroEngine::Device
{
    class RenderingDevice;
}

namespace GyroEngine::Resources
{

    struct DescriptorSetLayoutInfo
    {
        uint32_t setNumber;
        std::vector<Utils::Shader::ShaderBinding> bindings;

        bool operator==(const DescriptorSetLayoutInfo& other) const {
            return setNumber == other.setNumber &&
                   bindings == other.bindings;
        }
    };

    struct DescriptorSetLayoutInfoHasher {
        std::size_t operator()(const DescriptorSetLayoutInfo& info) const {
            std::size_t h = std::hash<uint32_t>{}(info.setNumber);
            for (const auto& binding : info.bindings) {
                h ^= std::hash<uint32_t>{}(binding.binding) + 0x9e3779b9 + (h << 6) + (h >> 2);
                h ^= std::hash<uint32_t>{}(binding.type) + 0x9e3779b9 + (h << 6) + (h >> 2);
                h ^= std::hash<uint32_t>{}(binding.count) + 0x9e3779b9 + (h << 6) + (h >> 2);
                h ^= std::hash<uint32_t>{}(binding.stageFlags) + 0x9e3779b9 + (h << 6) + (h >> 2);
            }
            return h;
        }
    };

    class PipelineBindings
    {
    public:
        PipelineBindings(Device::RenderingDevice& device, Utils::Shader::ShaderReflection reflection)
            : m_device(device), m_reflection(std::move(reflection)) {}
        ~PipelineBindings() { Cleanup(); }

        bool Init();
        void Cleanup();

        void UpdateImageSet(uint32_t set, uint32_t binding, VkDescriptorType descriptorType, SamplerHandle sampler, ImageHandle image, uint32_t frameIndex);
        void UpdateImageSet(const std::string &bindingName, SamplerHandle sampler, ImageHandle image, uint32_t frameIndex);

        void UpdateBufferSet(uint32_t set, uint32_t binding, VkDescriptorType descriptorType, const BufferHandle &buffer, uint32_t frameIndex);
        void UpdateBufferSet(const std::string &bindingName, const BufferHandle &buffer, uint32_t frameIndex);

        void PushConstant(VkCommandBuffer cmd, VkPipelineLayout layout, VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void* data) const;
        void PushConstant(const std::string& block, const std::string& name, VkCommandBuffer cmd, VkPipelineLayout layout, const void* data);

        void BindSet(VkCommandBuffer cmd, VkPipelineBindPoint bindPoint, VkPipelineLayout layout, uint32_t set, uint32_t frameIndex);
        void BindSet(const std::string &bindingName, VkCommandBuffer cmd, VkPipelineBindPoint bindPoint, VkPipelineLayout layout, uint32_t frameIndex);

        bool DoesBindingExist(const std::string &bindingName) const;

        [[nodiscard]] std::vector<VkDescriptorSetLayout> GetDescriptorSetLayouts() const
        {
            std::vector<VkDescriptorSetLayout> layouts;
            for (const auto& [info, layout] : layoutMap)
            {
                layouts.push_back(layout);
            }
            return layouts;
        }

        [[nodiscard]] Utils::Shader::ShaderReflection GetReflection() const
        {
            return m_reflection;
        }
    private:
        Device::RenderingDevice& m_device;

        Utils::Shader::ShaderReflection m_reflection;
        std::unordered_map<DescriptorSetLayoutInfo, VkDescriptorSetLayout, DescriptorSetLayoutInfoHasher> layoutMap;
        VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
        std::unordered_map<uint32_t, std::vector<VkDescriptorSet>> m_descriptorSets;

        bool CreateDescriptorSetLayouts();
        void DestroyDescriptorSetLayouts();

        bool CreateDescriptorPool();
        void DestroyDescriptorPool();

        bool AllocateDescriptorSets();
        void FreeDescriptorSets();
    };

}
