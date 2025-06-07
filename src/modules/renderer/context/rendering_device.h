//
// Created by lepag on 6/7/2025.
//

#pragma once

#include <SDL3/SDL.h>
#include <VkBootstrap.h>

#include "implementation/volk_implementation.h"

enum class DeviceQueueType
{
    None,
    Graphics,
    Compute,
    Transfer,
    Present
};

/// @brief GPU queue that is used for submitting rendering commands.
struct DeviceQueue
{
    uint32_t family = 0;
    DeviceQueueType type = DeviceQueueType::None;
    VkQueue queue = VK_NULL_HANDLE;

    [[nodiscard]] bool isValid() const {
        return queue != VK_NULL_HANDLE;
    }
};

/// @brief Collection of device queues for different operations.
struct DeviceFamilies
{
    std::vector<DeviceQueue> queues;

    [[nodiscard]] bool isComplete() const
    {
        for (const auto& queue : queues) {
            if (!queue.isValid()) {
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] DeviceQueue getQueue(const DeviceQueueType type) const
    {
        for (const auto& queue : queues) {
            if (queue.type == type) {
                return queue;
            }
        }
        return {};
    }

    [[nodiscard]] DeviceQueue getGraphicsQueue() const
    {
        return getQueue(DeviceQueueType::Graphics);
    }

    [[nodiscard]] DeviceQueue getComputeQueue() const
    {
        return getQueue(DeviceQueueType::Compute);
    }

    [[nodiscard]] DeviceQueue getTransferQueue() const
    {
        return getQueue(DeviceQueueType::Transfer);
    }

    [[nodiscard]] DeviceQueue getPresentQueue() const
    {
        return getQueue(DeviceQueueType::Present);
    }
};

/// @brief Encapsulates management of rendering objects and rendering operations.
class RenderingDevice {
public:
    RenderingDevice();
    ~RenderingDevice();

    bool init();
    void cleanup();

    [[nodiscard]] VkInstance getInstance() const {
        return m_instance;
    }

    [[nodiscard]] VkPhysicalDevice getPhysicalDevice() const {
        return m_physicalDevice;
    }

    [[nodiscard]] VkDevice getLogicalDevice() const {
        return m_logicalDevice;
    }

    [[nodiscard]] DeviceFamilies& getDeviceFamilies() {
        return m_deviceFamilies;
    }
private:
    VkInstance m_instance = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_logicalDevice = VK_NULL_HANDLE;
    DeviceFamilies m_deviceFamilies;
};
