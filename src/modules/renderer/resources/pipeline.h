//
// Created by lepag on 6/10/2025.
//

#pragma once

#include <volk.h>

#include "descriptor_manager.h"
#include "device_resource.h"
#include "utilities/pipeline.h"


class Pipeline : IDeviceResource {
public:
    explicit Pipeline(RenderingDevice& device): IDeviceResource(device) {}
    ~Pipeline() override = default;

    Pipeline& setDescriptorManager(const std::shared_ptr<DescriptorManager>& descriptorManager);
    Pipeline& clearConfig();
    Pipeline& setColorFormat(VkFormat colorFormat);

    bool init() override;
    void cleanup() override;

    void bind(const FrameContext& frameContext);

    [[nodiscard]] pipelineutils::PipelineConfig& getPipelineConfig()
    {
        return m_pipelineConfig;
    }
private:
    VkPipeline m_pipeline = VK_NULL_HANDLE;
    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    std::shared_ptr<DescriptorManager> m_descriptorManager;

    pipelineutils::PipelineConfig m_pipelineConfig{};

    bool buildPipelineLayout();
    bool buildPipeline();
};
