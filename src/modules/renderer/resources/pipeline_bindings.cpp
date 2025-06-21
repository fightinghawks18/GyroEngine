//
// Created by lepag on 6/19/2025.
//

#include "pipeline_bindings.h"

#include <map>

#include "context/rendering_device.h"
#include "utilities/pipeline.h"

namespace GyroEngine::Resources
{
    bool PipelineBindings::Init()
    {
        if (!CreateDescriptorSetLayouts()) return false;
        if (!CreateDescriptorPool()) return false;
        if (!AllocateDescriptorSets()) return false;
        return true;
    }

    void PipelineBindings::Cleanup()
    {
        FreeDescriptorSets();
        DestroyDescriptorPool();
        DestroyDescriptorSetLayouts();
    }

    void PipelineBindings::UpdateImageSet(uint32_t set, uint32_t binding, VkDescriptorType descriptorType, SamplerHandle sampler, ImageHandle image, uint32_t frameIndex)
    {
        VkDescriptorImageInfo imageInfo = {};
        imageInfo.sampler = sampler ? sampler->GetSampler() : VK_NULL_HANDLE;
        imageInfo.imageView = image ? image->GetImageView() : VK_NULL_HANDLE;
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkDescriptorSet descriptorSet = m_descriptorSets[set][frameIndex];
        const auto imageWriteInfo = Utils::Pipeline::MakeWriteDescriptorImage(descriptorSet, binding, descriptorType, &imageInfo);

        vkUpdateDescriptorSets(m_device.GetLogicalDevice(), 1, &imageWriteInfo, 0, nullptr);
    }

    void PipelineBindings::UpdateImageSet(const std::string &bindingName, SamplerHandle sampler, ImageHandle image, uint32_t frameIndex)
    {
        Utils::Shader::ShaderBinding bindingInfo{};
        for (const auto &binding: m_reflection.descriptorSets)
        {
            if (binding.name == bindingName &&
                (binding.type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ||
                 binding.type == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE ||
                 binding.type == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE))
            {
                bindingInfo = binding;
                break;
            }
        }
        if (bindingInfo.name.empty())
        {
            Logger::LogError("No image binding found for binding name: " + bindingName);
            return;
        }

        VkDescriptorImageInfo imageInfo = {};
        imageInfo.sampler = sampler ? sampler->GetSampler() : VK_NULL_HANDLE;
        imageInfo.imageView = image ? image->GetImageView() : VK_NULL_HANDLE;
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkDescriptorSet descriptorSet = m_descriptorSets[bindingInfo.set][frameIndex];
        const auto imageWriteInfo = Utils::Pipeline::MakeWriteDescriptorImage(descriptorSet, bindingInfo.binding, bindingInfo.type, &imageInfo);

        vkUpdateDescriptorSets(m_device.GetLogicalDevice(), 1, &imageWriteInfo, 0, nullptr);
    }


    void PipelineBindings::UpdateBufferSet(uint32_t set, uint32_t binding, VkDescriptorType descriptorType, const BufferHandle &buffer, const uint32_t frameIndex)
    {
        VkDescriptorBufferInfo bufferInfo = {};
        bufferInfo.buffer = buffer ? buffer->GetBuffer() : VK_NULL_HANDLE;
        bufferInfo.offset = 0;
        bufferInfo.range = buffer ? buffer->GetSize() : 0;

        VkDescriptorSet descriptorSet = m_descriptorSets[set][frameIndex];
        const auto bufferWriteInfo = Utils::Pipeline::MakeWriteDescriptorBuffer(descriptorSet, binding, descriptorType, &bufferInfo);

        vkUpdateDescriptorSets(m_device.GetLogicalDevice(), 1, &bufferWriteInfo, 0, nullptr);
    }

    void PipelineBindings::UpdateBufferSet(const std::string &bindingName, const BufferHandle &buffer, const uint32_t frameIndex)
    {
        Utils::Shader::ShaderBinding bindingInfo{};
        for (const auto &binding: m_reflection.descriptorSets)
        {
            if (binding.name == bindingName &&
                (binding.type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ||
                 binding.type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER))
            {
                bindingInfo = binding;
                break;
            }
        }
        if (bindingInfo.name.empty())
        {
            Logger::LogError("No buffer binding found for binding name: " + bindingName);
            return;
        }

        VkDescriptorBufferInfo bufferInfo = {};
        bufferInfo.buffer = buffer ? buffer->GetBuffer() : VK_NULL_HANDLE;
        bufferInfo.offset = 0;
        bufferInfo.range = buffer ? buffer->GetSize() : 0;
        VkDescriptorSet descriptorSet = m_descriptorSets[bindingInfo.set][frameIndex];

        const auto bufferWriteInfo = Utils::Pipeline::MakeWriteDescriptorBuffer(descriptorSet, bindingInfo.binding, bindingInfo.type, &bufferInfo);
        vkUpdateDescriptorSets(m_device.GetLogicalDevice(), 1, &bufferWriteInfo, 0, nullptr);
    }

    void PipelineBindings::PushConstant(VkCommandBuffer cmd, VkPipelineLayout layout, VkShaderStageFlags stageFlags,
        uint32_t offset, uint32_t size, const void *data) const
    {
        vkCmdPushConstants(cmd, layout, stageFlags, offset, size, data);
    }

    void PipelineBindings::PushConstant(const std::string& block, const std::string &name, VkCommandBuffer cmd, VkPipelineLayout layout,
        const void *data)
    {
        Utils::Shader::ShaderPushConstantMember memberInfo{};
        VkShaderStageFlags stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        for (const auto &pc: m_reflection.pushConstants)
        {
            if (pc.name == block)
            {
                for (const auto &member: pc.members)
                {
                    if (member.name == name)
                    {
                        memberInfo = member;
                        stageFlags = pc.stage;
                        break;
                    }
                }
            }
        }
        if (memberInfo.name.empty())
        {
            Logger::LogError("No push constant found for name: " + name);
            return;
        }
        vkCmdPushConstants(cmd, layout, stageFlags, memberInfo.offset, memberInfo.size, data);
    }

    void PipelineBindings::BindSet(VkCommandBuffer cmd, VkPipelineBindPoint bindPoint, VkPipelineLayout layout, uint32_t set, uint32_t frameIndex)
    {
        vkCmdBindDescriptorSets(cmd, bindPoint, layout, set, 1, &m_descriptorSets[set][frameIndex], 0, nullptr);
    }

    void PipelineBindings::BindSet(const std::string &bindingName, VkCommandBuffer cmd, VkPipelineBindPoint bindPoint, VkPipelineLayout layout, uint32_t frameIndex)
    {
        Utils::Shader::ShaderBinding bindingInfo{};
        for (const auto &binding: m_reflection.descriptorSets)
        {
            if (binding.name == bindingName)
            {
                bindingInfo = binding;
                break;
            }
        }
        if (bindingInfo.name.empty())
        {
            Logger::LogError("No binding found for binding name: " + bindingName);
            return;
        }
        vkCmdBindDescriptorSets(cmd, bindPoint, layout, bindingInfo.set, 1, &m_descriptorSets[bindingInfo.set][frameIndex], 0, nullptr);
    }

    bool PipelineBindings::DoesBindingExist(const std::string &bindingName) const
    {
        for (const auto &binding: m_reflection.descriptorSets)
        {
            if (binding.name == bindingName)
            {
                return true;
            }
        }
        return false;
    }

    bool PipelineBindings::DoesPushConstantExist(const std::string &blockName) const
    {
        for (const auto &block: m_reflection.pushConstants)
        {
            if (block.name == blockName)
            {
                return true;
            }
        }
        return false;
    }

    bool PipelineBindings::CreateDescriptorSetLayouts()
    {
        std::unordered_map<uint32_t, std::vector<Utils::Shader::ShaderBinding>> bindingsPerSet;
        for (const auto& binding : m_reflection.descriptorSets) {
            bindingsPerSet[binding.set].push_back(binding);
        }

        for (const auto& [setNumber, bindings] : bindingsPerSet) {
            DescriptorSetLayoutInfo layoutInfo;
            layoutInfo.setNumber = setNumber;
            layoutInfo.bindings = bindings;

            std::map<std::pair<uint32_t, uint32_t>, VkDescriptorSetLayoutBinding> mergedBindings;
            std::vector<VkDescriptorSetLayoutBinding> vkBindings;
            for (const auto& b : bindings) {
                auto key = std::make_pair(b.set, b.binding);
                auto it = mergedBindings.find(key);
                if (it == mergedBindings.end()) {
                    VkDescriptorSetLayoutBinding vkBinding = {};
                    vkBinding.binding = b.binding;
                    vkBinding.descriptorType = b.type;
                    vkBinding.stageFlags = b.stageFlags;
                    vkBinding.descriptorCount = b.count;
                    vkBinding.pImmutableSamplers = nullptr;
                    mergedBindings[key] = vkBinding;
                } else {
                    it->second.stageFlags |= b.stageFlags;
                }
            }

            for (auto& vkBinding : mergedBindings)
            {
                vkBindings.push_back(vkBinding.second);
            }

            VkDescriptorSetLayoutCreateInfo layoutCI = {};
            layoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layoutCI.bindingCount = static_cast<uint32_t>(vkBindings.size());
            layoutCI.pBindings = vkBindings.data();

            VkDescriptorSetLayout layout = VK_NULL_HANDLE;
            if (vkCreateDescriptorSetLayout(m_device.GetLogicalDevice(), &layoutCI, nullptr, &layout) != VK_SUCCESS) {
                Logger::LogError("Failed to create descriptor set layout for set " + std::to_string(setNumber));
                return false;
            }

            layoutMap[layoutInfo] = layout;
        }
        return true;
    }

    void PipelineBindings::DestroyDescriptorSetLayouts()
    {
        if (!layoutMap.empty())
        {
            for (const auto& [info, layout] : layoutMap)
            {
                vkDestroyDescriptorSetLayout(m_device.GetLogicalDevice(), layout, nullptr);
            }
            layoutMap.clear();
        }
    }

    bool PipelineBindings::CreateDescriptorPool()
    {
        std::unordered_map<uint32_t, std::vector<Utils::Shader::ShaderBinding>> bindingsPerSet;
        for (const auto& binding : m_reflection.descriptorSets)
            bindingsPerSet[binding.set].push_back(binding);

        std::unordered_map<VkDescriptorType, uint32_t> typeCounts;
        for (const auto& [setNumber, bindings] : bindingsPerSet)
        {
            for (const auto& binding : bindings)
                typeCounts[binding.type] += binding.count;
        }

        // 3. Create pool sizes array
        std::vector<VkDescriptorPoolSize> poolSizes;
        uint32_t frames = m_device.GetMaxFramesInFlight();
        for (const auto& [type, count] : typeCounts)
        {
            VkDescriptorPoolSize sz = {};
            sz.type = type;
            sz.descriptorCount = count * frames;
            poolSizes.push_back(sz);
        }

        VkDescriptorPoolCreateInfo poolCI = {};
        poolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolCI.maxSets = static_cast<uint32_t>(bindingsPerSet.size() * frames);
        poolCI.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolCI.pPoolSizes = poolSizes.data();

        if (vkCreateDescriptorPool(m_device.GetLogicalDevice(), &poolCI, nullptr, &m_descriptorPool) != VK_SUCCESS)
        {
            Logger::LogError("Failed to create descriptor pool.");
            return false;
        }
        return true;
    }

    void PipelineBindings::DestroyDescriptorPool()
    {
        if (m_descriptorPool != VK_NULL_HANDLE)
        {
            vkDestroyDescriptorPool(m_device.GetLogicalDevice(), m_descriptorPool, nullptr);
            m_descriptorPool = VK_NULL_HANDLE;
        }
    }

    bool PipelineBindings::AllocateDescriptorSets()
    {
        // 1. Group bindings by set number
        std::unordered_map<uint32_t, std::vector<Utils::Shader::ShaderBinding>> bindingsPerSet;
        for (const auto& binding : m_reflection.descriptorSets)
            bindingsPerSet[binding.set].push_back(binding);

        const uint32_t frames = m_device.GetMaxFramesInFlight();
        for (const auto& [setNumber, bindings] : bindingsPerSet)
        {
            DescriptorSetLayoutInfo info;
            info.setNumber = setNumber;
            info.bindings = bindings;

            auto it = layoutMap.find(info);
            if (it == layoutMap.end())
            {
                Logger::LogError("Descriptor set layout missing for set " + std::to_string(setNumber));
                return false;
            }
            VkDescriptorSetLayout layout = it->second;

            std::vector layouts(frames, layout);

            VkDescriptorSetAllocateInfo allocInfo = {};
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorPool = m_descriptorPool;
            allocInfo.descriptorSetCount = frames;
            allocInfo.pSetLayouts = layouts.data();

            std::vector<VkDescriptorSet> sets(frames);
            if (vkAllocateDescriptorSets(m_device.GetLogicalDevice(), &allocInfo, sets.data()) != VK_SUCCESS)
            {
                Logger::LogError("Failed to allocate descriptor sets for set " + std::to_string(setNumber));
                return false;
            }
            m_descriptorSets[setNumber] = std::move(sets);
        }
        return true;
    }

    void PipelineBindings::FreeDescriptorSets()
    {
        m_descriptorSets.clear();
    }
}
