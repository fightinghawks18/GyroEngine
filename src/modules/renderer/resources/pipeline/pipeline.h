//
// Created by lepag on 6/10/2025.
//

#pragma once

#include <memory>
#include <volk.h>

#include "pipeline_bindings.h"
#include "utilities/pipeline.h"

namespace GyroEngine::Device
{
    class RenderingDevice;
}

namespace GyroEngine::Rendering
{
    struct FrameContext;
}

namespace GyroEngine::Resources
{
    class Pipeline {
    public:
        explicit Pipeline(Device::RenderingDevice& device): m_device(device) {}
        ~Pipeline() { Cleanup(); }

        Pipeline& SetPipelineBindings(const std::shared_ptr<PipelineBindings>& pipelineBindings);
        Pipeline& ClearConfig();
        Pipeline& SetColorFormat(VkFormat colorFormat);

        bool Init();
        void Cleanup();

        void Bind(const Rendering::FrameContext& frameContext) const;
        void DrawFullscreenQuad(const Rendering::FrameContext& frameContext) const;

        [[nodiscard]] Utils::Pipeline::PipelineConfig& GetPipelineConfig()
        {
            return m_pipelineConfig;
        }

        [[nodiscard]] VkPipeline GetPipeline() const
        {
            return m_pipeline;
        }

        [[nodiscard]] VkPipelineLayout GetPipelineLayout() const
        {
            return m_pipelineLayout;
        }

        [[nodiscard]] std::shared_ptr<PipelineBindings> GetPipelineBindings() const
        {
            return m_pipelineBindings;
        }
    private:
        Device::RenderingDevice& m_device;

        VkPipeline m_pipeline = VK_NULL_HANDLE;
        VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
        std::shared_ptr<PipelineBindings> m_pipelineBindings;

        Utils::Pipeline::PipelineConfig m_pipelineConfig{};

        // Using a local member causes memory violations
        // ^ Why did I choose to create a member to temp store them?
        // ^^ I have no clue, but it works
        std::vector<VkDescriptorSetLayout> m_descriptorSetLayouts;
        std::vector<VkPushConstantRange> m_pushConstantRanges;

        bool BuildPipelineLayout();
        bool BuildPipeline();
    };

    using PipelineHandle = std::shared_ptr<Pipeline>;
}
