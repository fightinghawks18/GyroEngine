//
// Created by lepag on 6/7/2025.
//

#pragma once

#include <any>
#include <memory>

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#include "../../platform/window.h"
#include "tasks/maid.h"

#include "implementation/volk_implementation.h"
#include "implementation/vma_implementation.h"
#include "utilities/device.h"


namespace GyroEngine::Device
{
    /// @brief GPU queue that is used for submitting rendering commands.
    struct DeviceQueue
    {
        uint32_t family = 0;
        Utils::Device::QueueType type = Utils::Device::QueueType::None;
        VkQueue queue = VK_NULL_HANDLE;

        [[nodiscard]] bool isValid() const
        {
            return queue != VK_NULL_HANDLE;
        }
    };

    /// @brief Collection of device queues for different operations.
    struct DeviceFamilies
    {
        std::vector<DeviceQueue> queues;

        [[nodiscard]] bool IsComplete() const
        {
            for (const auto &queue: queues)
            {
                if (!queue.isValid())
                {
                    return false;
                }
            }
            return true;
        }

        [[nodiscard]] DeviceQueue GetQueue(const Utils::Device::QueueType type) const
        {
            for (const auto &queue: queues)
            {
                if (queue.type == type)
                {
                    return queue;
                }
            }
            return {};
        }

        [[nodiscard]] DeviceQueue GetGraphicsQueue() const
        {
            return GetQueue(Utils::Device::QueueType::Graphics);
        }

        [[nodiscard]] DeviceQueue GetComputeQueue() const
        {
            return GetQueue(Utils::Device::QueueType::Compute);
        }

        [[nodiscard]] DeviceQueue GetTransferQueue() const
        {
            return GetQueue(Utils::Device::QueueType::Transfer);
        }

        [[nodiscard]] DeviceQueue GetPresentQueue() const
        {
            return GetQueue(Utils::Device::QueueType::Present);
        }
    };

    enum class PreferredColorFormatType
    {
        sRGB,
        uNORM,
        HDR,
        LINEAR,
        COMPATIBLE,
        DEFAULT
    };

    /// @brief Encapsulates management of rendering objects and rendering operations.
    class RenderingDevice
    {
    public:
        RenderingDevice();

        ~RenderingDevice();

        bool Init();

        void Cleanup();

        void AddCleanupTask(const std::function<void()> &task) { m_maid.Add(task); }
        void AllowDiscrete(const bool allow = true) { m_acceptDiscrete = allow; }
        void AllowIntegrated(const bool allow = true) { m_acceptIntegrated = allow; }
        void AllowCPU(const bool allow = true) { m_acceptCPU = allow; }
        void RequireTesselation(const bool require = true) { m_requiresTesselation = require; }

        void WaitForIdle() const
        {
            vkDeviceWaitIdle(m_logicalDevice);
        }

        void SetColorPreference(PreferredColorFormatType preferredColorFormat);

        void SetSwapchainColorFormat(VkFormat swapchainColorFormat);

        VkFormat QueryForSupportedColorFormat(VkFormat format);

        VkFormat QueryForSupportedDepthFormat(VkFormat format);

        VkSurfaceKHR CreateSurfaceFromWindow(const Window *window) const;

        DeviceQueue GetPresentQueueFromSurface(VkSurfaceKHR surface) const;

        [[nodiscard]] VkInstance GetInstance() const
        {
            return m_instance;
        }

        [[nodiscard]] VkPhysicalDevice GetPhysicalDevice() const
        {
            return m_physicalDevice;
        }

        [[nodiscard]] VkPhysicalDeviceProperties GetPhysicalDeviceProperties() const
        {
            VkPhysicalDeviceProperties properties;
            vkGetPhysicalDeviceProperties(m_physicalDevice, &properties);
            return properties;
        }

        [[nodiscard]] VkPhysicalDeviceFeatures GetPhysicalDeviceFeatures() const
        {
            VkPhysicalDeviceFeatures features;
            vkGetPhysicalDeviceFeatures(m_physicalDevice, &features);
            return features;
        }

        [[nodiscard]] VkDevice GetLogicalDevice() const
        {
            return m_logicalDevice;
        }

        [[nodiscard]] VmaAllocator GetAllocator() const
        {
            return m_allocator;
        }

        [[nodiscard]] VkCommandPool GetCommandPool() const
        {
            return m_commandPool;
        }

        [[nodiscard]] VkFormat GetSwapchainFormat() const
        {
            return m_swapchainColorFormat;
        }

        [[nodiscard]] VkFormat GetPreferredColorFormat() const
        {
            return m_colorFormat;
        }

        [[nodiscard]] VkFormat GetPreferredDepthFormat() const
        {
            return m_depthFormat;
        }

        [[nodiscard]] VkFormat GetPreferredStencilFormat() const
        {
            return m_stencilFormat;
        }

        [[nodiscard]] const std::vector<VkFormat> &GetSupportedColorFormats() const
        {
            return m_supportedColorFormats;
        }

        [[nodiscard]] const std::vector<VkFormat> &GetSupportedDepthFormats() const
        {
            return m_supportedDepthFormats;
        }

        [[nodiscard]] uint32_t GetMaxFramesInFlight() const
        {
            return m_maxFramesInFlight;
        }

        [[nodiscard]] Maid &GetMaid()
        {
            return m_maid;
        }

        [[nodiscard]] DeviceFamilies &GetDeviceFamilies()
        {
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

        uint32_t m_maxFramesInFlight = 2;
        PreferredColorFormatType m_preferredColorType = PreferredColorFormatType::sRGB;
        VkFormat m_colorFormat = VK_FORMAT_UNDEFINED;
        VkFormat m_swapchainColorFormat = VK_FORMAT_UNDEFINED;
        VkFormat m_depthFormat = VK_FORMAT_UNDEFINED;
        VkFormat m_stencilFormat = VK_FORMAT_UNDEFINED;

        // Setup configuration

        bool m_requiresTesselation;
        bool m_acceptDiscrete;
        bool m_acceptIntegrated;
        bool m_acceptCPU;

        // Device setup/cleanup

        bool CreateInstance();

        bool SetupDebugMessenger();

        bool SelectPhysicalDevice();

        bool CreateLogicalDevice();

        bool CreateDeviceFamilies();

        bool CreateAllocator();

        bool CreateCommandPool();

        bool QueryAllSupportedColorFormats();

        bool QueryAllSupportedDepthFormats();

        void DestroyCommandPool();

        void DestroyAllocator();

        void DestroyLogicalDevice();

        void DestroyPhysicalDevice();

        void DestroyDebugMessenger();

        void DestroyInstance();

        // Device format helpers

        bool FindPreferredColorFormat();

        bool FindBestDepthFormat();

        // Device static helpers

        static VKAPI_ATTR VkBool32 VKAPI_CALL DebugMessengerCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
            void *pUserData);
    };
}
