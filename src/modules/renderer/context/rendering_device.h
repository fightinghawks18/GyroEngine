//
// Created by lepag on 6/7/2025.
//

#pragma once

#include <map>

#include "../../platform/window.h"
#include "tasks/maid.h"
#include "debug/printer.h"

#include "implementation/volk_implementation.h"
#include "implementation/vma_implementation.h"
#include "utilities/device.h"

/// @brief GPU queue that is used for submitting rendering commands.
struct DeviceQueue
{
    uint32_t family = 0;
    rendererutils::QueueType type = rendererutils::QueueType::None;
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

    [[nodiscard]] DeviceQueue getQueue(const rendererutils::QueueType type) const
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
        return getQueue(rendererutils::QueueType::Graphics);
    }

    [[nodiscard]] DeviceQueue getComputeQueue() const
    {
        return getQueue(rendererutils::QueueType::Compute);
    }

    [[nodiscard]] DeviceQueue getTransferQueue() const
    {
        return getQueue(rendererutils::QueueType::Transfer);
    }

    [[nodiscard]] DeviceQueue getPresentQueue() const
    {
        return getQueue(rendererutils::QueueType::Present);
    }
};

/// @brief Encapsulates management of rendering objects and rendering operations.
class RenderingDevice {
public:
    RenderingDevice();
    ~RenderingDevice();

    bool init();
    void cleanup();
    void addCleanup(const std::function<void()> &task) { m_maid.add(task); }
    void allowDiscrete(const bool allow = true) { m_acceptDiscrete = allow; }
    void allowIntegrated(const bool allow = true) { m_acceptIntegrated = allow; }
    void allowCPU(const bool allow = true) { m_acceptCPU = allow; }
    void requireTesselation(const bool require = true) { m_requiresTesselation = require; }
    void waitIdle() const {
        vkDeviceWaitIdle(m_logicalDevice);
    }

    VkFormat queryColorFormat(VkFormat format);
    VkFormat queryDepthFormat(VkFormat format);
    VkFormat queryStencilFormat(VkFormat format);

    VkSurfaceKHR createSurface(const Window* window) const;
    DeviceQueue getPresentQueue(VkSurfaceKHR surface) const;

    [[nodiscard]] VkInstance getInstance() const {
        return m_instance;
    }

    [[nodiscard]] VkPhysicalDevice getPhysicalDevice() const {
        return m_physicalDevice;
    }

    [[nodiscard]] VkPhysicalDeviceProperties getPhysicalDeviceProperties() const
    {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(m_physicalDevice, &properties);
        return properties;
    }

    [[nodiscard]] VkPhysicalDeviceFeatures getPhysicalDeviceFeatures() const
    {
        VkPhysicalDeviceFeatures features;
        vkGetPhysicalDeviceFeatures(m_physicalDevice, &features);
        return features;
    }

    [[nodiscard]] VkDevice getLogicalDevice() const {
        return m_logicalDevice;
    }

    [[nodiscard]] VmaAllocator getAllocator() const
    {
        return m_allocator;
    }

    [[nodiscard]] VkCommandPool getCommandPool() const {
        return m_commandPool;
    }

    [[nodiscard]] const std::vector<VkFormat>& getSupportedColorFormats() const {
        return m_supportedColorFormats;
    }

    [[nodiscard]] const std::vector<VkFormat>& getSupportedDepthFormats() const {
        return m_supportedDepthFormats;
    }

    [[nodiscard]] const std::vector<VkFormat>& getSupportedStencilFormats() const {
        return m_supportedStencilFormats;
    }

    [[nodiscard]] uint32_t getMaxFramesInFlight() const {
        return m_maxFramesInFlight;
    }

    [[nodiscard]] Maid& getMaid() {
        return m_maid;
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
    VmaAllocator m_allocator = VK_NULL_HANDLE;
    VkCommandPool m_commandPool = VK_NULL_HANDLE;
    DeviceFamilies m_deviceFamilies;
    Maid m_maid;

    std::vector<VkFormat> m_supportedColorFormats;
    std::vector<VkFormat> m_supportedDepthFormats;
    std::vector<VkFormat> m_supportedStencilFormats;

    uint32_t m_maxFramesInFlight = 2;

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
    bool createAllocator();
    bool createCommandPool();
    bool querySupportedColorFormats();
    bool querySupportedDepthFormats();
    bool querySupportedStencilFormats();

    void destroyCommandPool();
    void destroyAllocator();
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
