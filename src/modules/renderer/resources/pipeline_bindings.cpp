//
// Created by lepag on 6/21/2025.
//

#include "pipeline_bindings.h"

#include <map>
#include <spirv_reflect.h>

#include "context/rendering_device.h"
#include "debug/logger.h"
#include "rendering/renderer.h"
#include "utilities/shader.h"

namespace GyroEngine::Resources
{
    bool PipelineBindings::Init()
    {
        if (!CreateSpvModules())
        {
            Logger::LogError("Failed to create SPIR-V modules for pipeline bindings.");
            return false;
        }
        if (!CreateDescriptorSetLayouts())
        {
            Logger::LogError("Failed to create descriptor set layouts for pipeline bindings.");
            DestroySpvModules();
            return false;
        }

        if (!CreateDescriptorPool())
        {
            DestroyDescriptorSetLayouts();
            DestroySpvModules();
            Logger::LogError("Failed to create descriptor pool for pipeline bindings.");
            return false;
        }

        if (!CreatePushConstantRanges())
        {
            DestroyDescriptorSetLayouts();
            DestroySpvModules();
            DestroyDescriptorPool();
            DestroyPushConstantRanges();
            Logger::LogError("Failed to create push constant ranges for pipeline bindings.");
            return false;
        }

        if (!GetInputAttributes())
        {
            DestroyDescriptorSetLayouts();
            DestroySpvModules();
            DestroyDescriptorPool();
            DestroyPushConstantRanges();
            Logger::LogError("Failed to get input attributes for pipeline bindings.");
            return false;
        }

        // If we have pools available, then we can create descriptor sets
        if (!m_descriptorPools.empty())
        {
            if (!AllocateDescriptorSets())
            {
                DestroyDescriptorSetLayouts();
                DestroySpvModules();
                DestroyDescriptorPool();
                DestroyPushConstantRanges();
                DestroyDescriptorSets();
                Logger::LogError("Failed to create descriptor sets for pipeline bindings.");
                return false;
            }
        }
        return true;
    }

    void PipelineBindings::Cleanup()
    {
        DestroyDescriptorSets();
        DestroyDescriptorPool();
        DestroyDescriptorSetLayouts();
        DestroySpvModules();
        m_shaderStages.clear();
        m_pushConstants.clear();
        m_sets.clear();
        m_spvModules.clear();
        m_descriptorPools.clear();
    }

    void PipelineBindings::UpdateDescriptorBuffer(const std::string &name, const BufferHandle &buffer,
                                                  uint32_t index)
    {
        auto bindingOpt = GetBinding(name);
        if (!bindingOpt.has_value())
        {
            Logger::LogError("Buffer binding {} was not found", name);
            return;
        }

        VkDescriptorBufferInfo bufferInfoDesc = {};
        bufferInfoDesc.buffer = buffer->GetBuffer();
        bufferInfoDesc.offset = 0;
        bufferInfoDesc.range = buffer->GetSize();

        VkWriteDescriptorSet writeDesc = {};
        writeDesc.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDesc.dstSet = bindingOpt->first.descriptorSets[index].descriptorSet;
        writeDesc.dstBinding = bindingOpt->second.binding;
        writeDesc.dstArrayElement = 0;
        writeDesc.descriptorType = bindingOpt->second.type;
        writeDesc.descriptorCount = 1;
        writeDesc.pBufferInfo = &bufferInfoDesc;

        vkUpdateDescriptorSets(m_device.GetLogicalDevice(), 1, &writeDesc, 0, nullptr);
        //Logger::Log("Updated descriptor buffer for binding: {}", name);
    }

    void PipelineBindings::UpdateDescriptorImage(const std::string &name, const SamplerHandle &sampler,
                                                 const ImageHandle &image,
                                                 uint32_t index)
    {
        const auto bindingOpt = GetBinding(name);
        if (!bindingOpt.has_value())
        {
            Logger::LogError("Image binding {} was not found", name);
            return;
        }

        VkDescriptorImageInfo imageInfoDesc = {};
        imageInfoDesc.imageLayout = image->GetImageLayout();
        imageInfoDesc.imageView = image->GetImageView();
        imageInfoDesc.sampler = sampler->GetSampler();

        VkWriteDescriptorSet writeDesc = {};
        writeDesc.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDesc.dstSet = bindingOpt->first.descriptorSets[index].descriptorSet;
        writeDesc.dstBinding = bindingOpt->second.binding;
        writeDesc.dstArrayElement = 0;
        writeDesc.descriptorType = bindingOpt->second.type;
        writeDesc.descriptorCount = 1;
        writeDesc.pImageInfo = &imageInfoDesc;

        vkUpdateDescriptorSets(m_device.GetLogicalDevice(), 1, &writeDesc, 0, nullptr);
        //Logger::Log("Updated descriptor image for binding: {}", name);
    }

    void PipelineBindings::UpdatePushConstant(const std::string &block, const std::string &name,
                                              const void *data, size_t size, uint32_t offset)
    {
        // Check for null data pointer first
        if (!data)
        {
            Logger::LogError("Null data pointer provided for push constant {} in block {}", name, block);
            return;
        }

        auto pushConstantOpt = GetPushConstant(block, name);
        if (!pushConstantOpt.has_value())
        {
            Logger::LogError("Push constant {} in block {} was not found", name, block);
            return;
        }

        auto &[b, m] = pushConstantOpt.value();
        uint32_t memberOffset = m.offset + offset;

        // Ensure the size does not exceed the push constant member size
        if (size + offset > m.size)
        {
            Logger::LogError("Push constant {} in block {} exceeds size limit", name, block);
            return;
        }

        // Allocate memory for the push constant data if not already allocated
        if (!m.data)
        {
            m.data = malloc(m.size);
            if (!m.data)
            {
                Logger::LogError("Failed to allocate memory for push constant {} in block {}", name, block);
                return;
            }
            memset(m.data, 0, m.size);
        }

        // Copy new data to the push constant data
        uint8_t *dataPtr = static_cast<uint8_t *>(m.data) + memberOffset;
        memcpy(dataPtr, data, size);
    }

    void PipelineBindings::BindConstants(const Rendering::FrameContext &frameContext, VkPipelineLayout pipelineLayout)
    {
        return; // No push constants to bind in this implementation
    }

    void PipelineBindings::Bind(const Rendering::FrameContext &frameContext, VkPipelineLayout pipelineLayout)
    {
        VkCommandBuffer cmd = frameContext.cmd;

        // Bind descriptor sets for each set
        for (const auto &set: m_sets)
        {
            if (set->descriptorSets.empty() || set->layout == VK_NULL_HANDLE)
                continue;

            uint32_t frameIndex = frameContext.frameIndex;
            // Ensure we don't go out of bounds if we have fewer descriptor sets than frames
            if (frameIndex >= set->descriptorSets.size())
                frameIndex = frameIndex % set->descriptorSets.size();

            VkDescriptorSet descriptorSet = set->descriptorSets[frameIndex].descriptorSet;
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
                                    set->set, 1, &descriptorSet, 0, nullptr);
        }
    }

    std::optional<std::pair<PipelineBindings::Set, PipelineBindings::Binding> > PipelineBindings::GetBinding(
        const std::string &name)
    {
        for (const auto &set: m_sets)
        {
            for (const auto &binding: set->bindings)
            {
                if (binding.name == name)
                {
                    return {{*set, binding}};
                }
            }
        }
        return std::nullopt;
    }

    std::optional<std::pair<PipelineBindings::PushConstantBlock, PipelineBindings::PushConstantMember> >
    PipelineBindings::GetPushConstant(const std::string &block, const std::string &name)
    {
        for (const auto &pushConstant: m_pushConstants)
        {
            if (pushConstant.name == block)
            {
                for (const auto &member: pushConstant.members)
                {
                    if (member.name == name)
                    {
                        return {{pushConstant, member}};
                    }
                }
            }
        }
        return std::nullopt;
    }

    bool PipelineBindings::CreateSpvModules()
    {
        // Enumerate through all shader stages and create SPIR-V modules to be reflected from
        for (const auto &stage: m_shaderStages)
        {
            // Reflect shader module
            SpvReflectShaderModule spvModule = {};

            auto spvData = Utils::Shader::ReadShaderSPV(stage->GetShaderPath());
            if (spvData.empty())
            {
                Logger::LogError("Failed to read SPIR-V data from shader: " + stage->GetShaderPath());
                return false;
            }

            SpvReflectResult result = spvReflectCreateShaderModule(spvData.size(), spvData.data(), &spvModule);
            if (result != SPV_REFLECT_RESULT_SUCCESS)
            {
                Logger::LogError("Failed to create SPIR-V reflection module for shader: " + stage->GetShaderPath());
                return false;
            }

            m_spvModules[stage] = spvModule;
        }
        return true;
    }

    void PipelineBindings::DestroySpvModules()
    {
        for (auto &[stage, spvModule]: m_spvModules)
        {
            spvReflectDestroyShaderModule(&spvModule);
            spvModule = {};
        }
    }

    bool PipelineBindings::GetInputAttributes()
    {
        // Iterate through each shader stage and reflect input attributes
        for (const auto &[stage, spvModule]: m_spvModules)
        {
            if (stage->GetShaderStage() != Utils::Shader::ShaderStage::Vertex)
            {
                // Input attributes are only relevant for vertex shaders
                continue;
            }

            uint32_t inputCount = 0;
            SpvReflectResult result = spvReflectEnumerateInputVariables(&spvModule, &inputCount, nullptr);
            if (result != SPV_REFLECT_RESULT_SUCCESS)
            {
                Logger::LogError("Failed to enumerate input variables for shader: " + stage->GetShaderPath());
                return false;
            }

            std::vector<SpvReflectInterfaceVariable *> inputs(inputCount);
            result = spvReflectEnumerateInputVariables(&spvModule, &inputCount, inputs.data());
            if (result != SPV_REFLECT_RESULT_SUCCESS)
            {
                Logger::LogError("Failed to enumerate input variables for shader: " + stage->GetShaderPath());
                return false;
            }

            // Populate input attributes
            for (const auto *input: inputs)
            {
                if ((input->decoration_flags & SPV_REFLECT_DECORATION_BUILT_IN) != 0) continue; // Skip built-in variables

                VertexInput attribute;
                attribute.name = input->name ? input->name : "UNKNOWN INPUT";
                attribute.format = static_cast<VkFormat>(input->format);
                attribute.location = input->location;
                attribute.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

                m_vertexInputs.push_back(attribute);
            }
        }
        return true;
    }

    bool PipelineBindings::CreateDescriptorSetLayouts()
    {
        // Iterate through each shader stage and setup the bindings
        for (auto &[stage, spvModule]: m_spvModules)
        {
            // Reflect shader's descriptor sets
            uint32_t setCount = 0;
            auto result = spvReflectEnumerateDescriptorSets(&spvModule, &setCount, nullptr);
            if (result != SPV_REFLECT_RESULT_SUCCESS)
            {
                spvReflectDestroyShaderModule(&spvModule);
                Logger::LogError("Failed to enumerate descriptor sets for shader {}", stage->GetShaderPath());
                return false;
            }

            // Populate vector with descriptor set information
            std::vector<SpvReflectDescriptorSet *> sets(setCount);
            result = spvReflectEnumerateDescriptorSets(&spvModule, &setCount, sets.data());

            // Iterate through each set and create bindings
            for (uint32_t i = 0; i < setCount; ++i)
            {
                std::shared_ptr<Set> set = nullptr;

                // Check if this set has already been created
                const SpvReflectDescriptorSet *spvSet = sets[i];
                auto it = std::find_if(m_sets.begin(), m_sets.end(),
                                       [&](const std::shared_ptr<Set> &thisSet)
                                       {
                                           return thisSet->set == spvSet->set;
                                       });
                if (it != m_sets.end())
                {
                    // Append this shader's stage to all bindings of this set
                    // This ensures that we can access the set from different shader stages (Vertex | Fragment | Compute, etc)
                    for (auto &binding: (*it)->bindings)
                    {
                        if (binding.binding == spvSet->set)
                        {
                            switch (stage->GetShaderStage())
                            {
                                case Utils::Shader::ShaderStage::Vertex:
                                    binding.layoutBinding.stageFlags |= VK_SHADER_STAGE_VERTEX_BIT;
                                    break;
                                case Utils::Shader::ShaderStage::Fragment:
                                    binding.layoutBinding.stageFlags |= VK_SHADER_STAGE_FRAGMENT_BIT;
                                    break;
                                default:
                                    binding.layoutBinding.stageFlags |= VK_SHADER_STAGE_ALL;
                                    break;
                            }
                        }
                    }
                    set = *it; // Use the existing set
                } else
                {
                    // Create a new set since we couldn't find one
                    set = std::make_shared<Set>();
                    set->set = spvSet->set;
                    set->layout = VK_NULL_HANDLE;
                    m_sets.push_back(set);
                }

                for (uint32_t j = 0; j < spvSet->binding_count; ++j)
                {
                    // Check if this binding hasn't been added already
                    auto bindingIt = std::find_if(set->bindings.begin(), set->bindings.end(),
                                                  [&](const Binding &b)
                                                  {
                                                      return b.binding == spvSet->bindings[j]->binding;
                                                  });
                    if (bindingIt != set->bindings.end())
                    {
                        // If it exists, we can skip adding it again
                        continue;
                    }
                    const SpvReflectDescriptorBinding *spvBinding = spvSet->bindings[j];
                    Binding binding;
                    binding.binding = spvBinding->binding;
                    binding.type = static_cast<VkDescriptorType>(spvBinding->descriptor_type);
                    binding.name = spvBinding->name ? spvBinding->name : "UNKNOWN SET";

                    switch (stage->GetShaderStage())
                    {
                        case Utils::Shader::ShaderStage::Vertex:
                            binding.layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
                            break;
                        case Utils::Shader::ShaderStage::Fragment:
                            binding.layoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
                            break;
                        default:
                            binding.layoutBinding.stageFlags = VK_SHADER_STAGE_ALL;
                            break;
                    }

                    // Create layout binding
                    binding.layoutBinding.binding = spvBinding->binding;
                    binding.layoutBinding.descriptorType = static_cast<VkDescriptorType>(spvBinding->descriptor_type);
                    binding.layoutBinding.descriptorCount = spvBinding->count;
                    binding.layoutBinding.stageFlags = binding.stageFlags;

                    // Add to set's bindings
                    set->bindings.push_back(binding);
                }
            }
            Logger::Log("Created descriptor set bindings for shader {} with {} sets", stage->GetShaderPath(), setCount);
        }

        // Iterate through all sets and create the layouts from all the bindings
        for (const auto& set : m_sets)
        {
            // Create the descriptor set layout
            VkDescriptorSetLayoutCreateInfo layoutInfo = {};
            layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layoutInfo.bindingCount = static_cast<uint32_t>(set->bindings.size());
            std::vector<VkDescriptorSetLayoutBinding> layoutBindings(set->bindings.size());
            for (size_t j = 0; j < set->bindings.size(); ++j)
            {
                layoutBindings[j] = set->bindings[j].layoutBinding;
            }

            layoutInfo.pBindings = reinterpret_cast<const VkDescriptorSetLayoutBinding *>(layoutBindings.data());

            if (VkResult res = vkCreateDescriptorSetLayout(m_device.GetLogicalDevice(), &layoutInfo, nullptr,
                                                           &set->layout); res != VK_SUCCESS)
            {
                Logger::LogError("Failed to create descriptor set layout for set {}", set->set);
                return false;
            }
            Logger::Log("Created layout for set {}, had {} bindings", set->set, set->bindings.size());
        }
        return true;
    }

    void PipelineBindings::DestroyDescriptorSetLayouts()
    {
        for (auto &set: m_sets)
        {
            if (set->layout != VK_NULL_HANDLE)
            {
                vkDestroyDescriptorSetLayout(m_device.GetLogicalDevice(), set->layout, nullptr);
                set->layout = VK_NULL_HANDLE;
            }
        }
    }

    bool PipelineBindings::CreatePushConstantRanges()
    {
        // Map to track push constants by name
        std::map<std::string, PushConstantBlock> pushConstantMap;

        // Enumerate all push constants from all shader stages
        for (auto &[stage, spvModule]: m_spvModules)
        {
            // Enumerate push constant blocks
            uint32_t pushConstantCount = 0;
            auto result = spvReflectEnumeratePushConstantBlocks(&spvModule, &pushConstantCount, nullptr);
            if (result != SPV_REFLECT_RESULT_SUCCESS)
            {
                Logger::LogError("Failed to enumerate push constants for shader: " + stage->GetShaderPath());
                return false;
            }

            if (pushConstantCount == 0) continue;

            // Populate vector with push constant information
            std::vector<SpvReflectBlockVariable *> pushConstants(pushConstantCount);
            result = spvReflectEnumeratePushConstantBlocks(&spvModule, &pushConstantCount, pushConstants.data());

            // Iterate through each push constant block
            for (uint32_t i = 0; i < pushConstantCount; ++i)
            {
                const SpvReflectBlockVariable *spvPushConstant = pushConstants[i];
                std::string blockName = spvPushConstant->name ? spvPushConstant->name : "default";

                VkShaderStageFlags stageFlag;
                switch (stage->GetShaderStage())
                {
                    case Utils::Shader::ShaderStage::Vertex:
                        stageFlag = VK_SHADER_STAGE_VERTEX_BIT;
                        break;
                    case Utils::Shader::ShaderStage::Fragment:
                        stageFlag = VK_SHADER_STAGE_FRAGMENT_BIT;
                        break;
                    default:
                        stageFlag = VK_SHADER_STAGE_ALL;
                        break;
                }

                // Check if this block already exists
                if (pushConstantMap.find(blockName) != pushConstantMap.end())
                {
                    // If exists, update the stage flags
                    pushConstantMap[blockName].stageFlags |= stageFlag;
                } else
                {
                    // Create new block
                    PushConstantBlock pushConstantBlock;
                    pushConstantBlock.name = blockName;
                    pushConstantBlock.stageFlags = stageFlag;
                    pushConstantBlock.offset = spvPushConstant->offset;
                    pushConstantBlock.size = spvPushConstant->size;

                    // Add members
                    for (uint32_t j = 0; j < spvPushConstant->member_count; ++j)
                    {
                        const SpvReflectBlockVariable *member = &spvPushConstant->members[j];
                        PushConstantMember pushConstantMember;
                        pushConstantMember.name = member->name ? member->name : "";
                        pushConstantMember.offset = member->offset;
                        pushConstantMember.size = member->size;
                        pushConstantMember.data = nullptr;
                        pushConstantBlock.members.push_back(pushConstantMember);
                    }

                    pushConstantMap[blockName] = pushConstantBlock;
                }
            }
        }

        // Convert map to vector
        m_pushConstants.clear();
        for (const auto &[name, block]: pushConstantMap)
        {
            m_pushConstants.push_back(block);
        }

        return true;
    }

    void PipelineBindings::DestroyPushConstantRanges()
    {
        // Enumerate through all push constants and free data from the members
        for (auto &pushConstant: m_pushConstants)
        {
            for (auto &member: pushConstant.members)
            {
                if (member.data)
                {
                    free(member.data);
                    member.data = nullptr;
                }
            }
            pushConstant.members.clear();
        }
    }

    bool PipelineBindings::CreateDescriptorPool()
    {
        // For every frame in flight we need to create a descriptor pool
        for (uint32_t i = 0; i < m_device.GetMaxFramesInFlight(); ++i)
        {
            std::unordered_map<VkDescriptorType, uint32_t> poolSizes;

            // Enumerate through all bindings to include all descriptor types
            for (const auto &set: m_sets)
            {
                for (const auto &binding: set->bindings)
                {
                    if (binding.type != VK_DESCRIPTOR_TYPE_MAX_ENUM)
                    {
                        // Increment the count for this descriptor type
                        poolSizes[binding.type] += binding.layoutBinding.descriptorCount;
                    }
                }
            }

            // Cannot create descriptor sets without any descriptor types
            if (poolSizes.empty())
            {
                Logger::Log("No descriptor types found for creating descriptor pool");
                continue;
            }

            // Convert the unordered_map to a vector of VkDescriptorPoolSize
            std::vector<VkDescriptorPoolSize> poolSizesVec;
            for (const auto &[type, count]: poolSizes)
            {
                VkDescriptorPoolSize poolSize = {};
                poolSize.type = type;
                poolSize.descriptorCount = count;
                poolSizesVec.push_back(poolSize);
            }

            VkDescriptorPoolCreateInfo poolInfo = {};
            poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
            poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizesVec.size());
            poolInfo.pPoolSizes = poolSizesVec.data();
            poolInfo.maxSets = static_cast<uint32_t>(m_sets.size());

            VkDescriptorPool descriptorPool;
            if (vkCreateDescriptorPool(m_device.GetLogicalDevice(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
            {
                Logger::LogError("Failed to create descriptor pool");
                return false;
            }

            m_descriptorPools.push_back(descriptorPool);
        }
        return true;
    }

    void PipelineBindings::DestroyDescriptorPool()
    {
        // Enumerate and destroy all descriptor pools
        for (auto &pool: m_descriptorPools)
        {
            if (pool != VK_NULL_HANDLE)
            {
                vkDestroyDescriptorPool(m_device.GetLogicalDevice(), pool, nullptr);
                pool = VK_NULL_HANDLE;
            }
        }
    }

    bool PipelineBindings::AllocateDescriptorSets() const
    {
        // Enumerate through all sets and allocate descriptor sets
        for (auto &set: m_sets)
        {
            // Create a set for each frame in flight
            for (uint32_t i = 0; i < m_device.GetMaxFramesInFlight(); ++i)
            {
                VkDescriptorSetLayout layout = set->layout;
                if (layout == VK_NULL_HANDLE)
                {
                    Logger::LogError("Descriptor set layout is not initialized for set {}", set->set);
                    return false;
                }

                VkDescriptorSetAllocateInfo allocInfo = {};
                allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                allocInfo.descriptorPool = m_descriptorPools[i];
                allocInfo.descriptorSetCount = 1;
                allocInfo.pSetLayouts = &layout;

                VkDescriptorSet descriptorSet;
                if (vkAllocateDescriptorSets(m_device.GetLogicalDevice(), &allocInfo, &descriptorSet) != VK_SUCCESS)
                {
                    Logger::LogError("Failed to allocate descriptor set for set {}", set->set);
                    return false;
                }

                // Add the allocated descriptor set to the set's collection
                set->descriptorSets.push_back({descriptorSet, m_descriptorPools[i]});
            }
        }
        return true;
    }

    void PipelineBindings::DestroyDescriptorSets() const
    {
        // Enumerate through all sets and destroy their descriptor sets
        for (auto &set: m_sets)
        {
            for (auto &[descriptorSet, descriptorPool]: set->descriptorSets)
            {
                if (descriptorSet != VK_NULL_HANDLE)
                {
                    vkFreeDescriptorSets(m_device.GetLogicalDevice(), descriptorPool, 1, &descriptorSet);
                    descriptorSet = VK_NULL_HANDLE;
                }
            }
            set->descriptorSets.clear();
        }
    }
}
