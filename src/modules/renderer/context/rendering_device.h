//
// Created by lepag on 6/7/2025.
//

#pragma once

#include <map>

#include "tasks/maid.h"
#include "debug/printer.h"

#include "implementation/volk_implementation.h"
#include "utilities/device.h"

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

    DeviceQueue getPresentQueue(VkSurfaceKHR surface);

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
    // Device objects

    VkInstance m_instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_logicalDevice = VK_NULL_HANDLE;
    DeviceFamilies m_deviceFamilies;
    Maid m_maid;

    // Setup configuration

    bool m_requiresTesselation = false;
    bool m_acceptDiscrete = true;
    bool m_acceptIntegrated = true;
    bool m_acceptCPU = true;

    // Device setup/cleanup

    bool createInstance();
    bool setupDebugMessenger();
    bool selectPhysicalDevice();
    bool createLogicalDevice();
    bool createDeviceFamilies();

    void destroyLogicalDevice();
    void destroyPhysicalDevice();
    void destroyDebugMessenger();
    void destroyInstance();

    // Device static helpers

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugMessengerCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData);
};
