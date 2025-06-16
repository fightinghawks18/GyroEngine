//
// Created by lepag on 6/10/2025.
//

#pragma once

#include <volk.h>

#include "descriptor_manager.h"
#include "device_resource.h"
#include "push_constant.h"
#include "utilities/pipeline.h"


class Pipeline : public IDeviceResource {
public:
    explicit Pipeline(RenderingDevice& device): IDeviceResource(device) {}
    ~Pipeline() override { Pipeline::cleanup(); }

    Pipeline& setDescriptorManager(const std::shared_ptr<DescriptorManager>& descriptorManager);
    Pipeline& clearConfig();
    Pipeline& setColorFormat(VkFormat colorFormat);

    bool init() override;
    void cleanup() override;

    void bind(const FrameContext& frameContext);
    void drawQuad(const FrameContext& frameContext);

    [[nodiscard]] pipelineutils::PipelineConfig& getPipelineConfig()
    {
        return m_pipelineConfig;
    }

    [[nodiscard]] VkPipeline getPipeline() const
    {
        return m_pipeline;
    }

    [[nodiscard]] VkPipelineLayout getPipelineLayout() const
    {
        return m_pipelineLayout;
    }

    [[nodiscard]] std::shared_ptr<DescriptorManager> getDescriptorManager() const
    {
        return m_descriptorManager;
    }
private:
    VkPipeline m_pipeline = VK_NULL_HANDLE;
    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    std::shared_ptr<DescriptorManager> m_descriptorManager;

    pipelineutils::PipelineConfig m_pipelineConfig{};

    // Using a local member causes memory violations
    // ^ Why did I choose to create a member to temp store them?
    // ^^ I have no clue, but it works
    std::vector<VkDescriptorSetLayout> m_descriptorSetLayouts;
    std::vector<VkPushConstantRange> m_pushConstantRanges;

    bool buildPipelineLayout();
    bool buildPipeline();
};
