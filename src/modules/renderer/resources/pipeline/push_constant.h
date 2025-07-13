//
// Created by lepag on 6/16/2025.
//

#pragma once

#include <cstring>
#include <volk.h>

namespace GyroEngine::Device
{
    class RenderingDevice;
}
using namespace GyroEngine;

namespace GyroEngine::Resources
{
    class PushConstant {
    public:
        PushConstant& SetSize(const uint32_t size) {
            m_size = size;
            return *this;
        }
        PushConstant& SetStageFlags(VkShaderStageFlags stageFlags) {
            m_stageFlags = stageFlags;
            return *this;
        }
        PushConstant& SetOffset(const uint32_t offset) {
            m_offset = offset;
            return *this;
        }

        void Set(const void* data, const uint32_t size, const uint32_t offset = 0)
        {
            if (m_data) delete[] static_cast<uint8_t*>(m_data);
            m_data = new uint8_t[size];
            memcpy(m_data, data, size);
            m_size = size;
            m_offset = offset;
        }

        void Push(VkCommandBuffer cmd, VkPipelineLayout pipelineLayout) const
        {
            vkCmdPushConstants(cmd, pipelineLayout, m_stageFlags, m_offset, m_size, m_data);
        }

        [[nodiscard]] uint32_t GetSize() const {
            return m_size;
        }
        [[nodiscard]] uint32_t GetOffset() const {
            return m_offset;
        }
        [[nodiscard]] VkShaderStageFlags GetStageFlags() const {
            return m_stageFlags;
        }
        [[nodiscard]] void* GetData() const {
            return m_data;
        }
    private:
        uint32_t m_size = 0;
        uint32_t m_offset = 0;
        VkShaderStageFlags m_stageFlags = VK_SHADER_STAGE_ALL;
        void* m_data = nullptr;
    };
}
