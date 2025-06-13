//
// Created by lepag on 6/7/2025.
//

#pragma once

#include <vector>
#include <SDL3/SDL_vulkan.h>

#include "debug/printer.h"

namespace deviceutils
{
    struct RankedDevice
    {
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        uint32_t score = 0;
        VkPhysicalDeviceProperties properties{};
    };

    enum class QueueType
    {
        None,
        Graphics,
        Compute,
        Transfer,
        Present
    };

    struct Extensions
    {
        uint32_t extensionCount = 0;
        std::vector<const char*> extensions;

        explicit operator std::vector<const char*>() const
        {
            return extensions;
        }

        [[nodiscard]] const char* const* data() const
        {
            return extensions.data();
        }

        explicit operator const char* const*() const
        {
            return data();
        }

        explicit operator uint32_t() const
        {
            return extensionCount;
        }
    };

    static Extensions getSDLExtensions()
    {
        Extensions exts;

        uint32_t extensionCount = 0;
        char const* const* sdlExtensions = SDL_Vulkan_GetInstanceExtensions(&extensionCount);

        exts.extensionCount = extensionCount;
        exts.extensions.reserve(extensionCount);
        for (uint32_t i = 0; i < extensionCount; ++i)
        {
            exts.extensions.push_back(sdlExtensions[i]);
        }
        return exts;
    }
    static Extensions createExtensions(const std::vector<const char*>& extensions)
    {
        Extensions exts;
        exts.extensionCount = extensions.size();
        exts.extensions.reserve(extensions.size());
        for (auto extension : extensions)
        {
            exts.extensions.push_back(extension);
        }
        return exts;
    }

    /// @note Returns a new vector containing only the supported device extensions from the provided list.
    static std::vector<const char*> enumerateVectorForSupportedDeviceExtensions(VkPhysicalDevice physicalDevice, const std::vector<const char*>& extensions)
    {
        uint32_t extensionCount = 0;
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());

        if (availableExtensions.empty())
        {
            Printer::error("No device extensions available");
            throw std::runtime_error("No device extensions available");
        }

        std::vector<const char*> supportedExtensions;
        for (const auto& requested : extensions)
        {
            bool found = false;
            for (const auto& available : availableExtensions)
            {
                if (strcmp(requested, available.extensionName) == 0)
                {
                    supportedExtensions.push_back(requested);
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                Printer::warn(requested + std::string(" is not supported by this device"));
            }
        }

        if (supportedExtensions.empty())
        {
            Printer::error("No supported device extensions found");
            throw std::runtime_error("No supported device extensions found");
        }

        return supportedExtensions;
    }

    /// @note Returns a new vector containing only the supported instance extensions from the provided list.
    static std::vector<const char*> enumerateVectorForSupportedInstanceExtensions(const std::vector<const char*>& extensions)
    {
        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());

        if (availableExtensions.empty())
        {
            Printer::error("No instance extensions available");
            throw std::runtime_error("No instance extensions available");
        }

        std::vector<const char*> supportedExtensions;

        for (const auto& requested : extensions)
        {
            bool found = false;
            for (const auto& available : availableExtensions)
            {
                if (strcmp(requested, available.extensionName) == 0)
                {
                    supportedExtensions.push_back(requested);
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                Printer::warn(requested + std::string(" is not supported by this instance"));
            }
        }

        // Check if any extensions were found
        if (supportedExtensions.empty())
        {
            Printer::error("No supported instance extensions found");
            throw std::runtime_error("No supported instance extensions found");
        }

        return supportedExtensions;
    }
}