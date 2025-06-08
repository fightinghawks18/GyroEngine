//
// Created by lepag on 6/7/2025.
//

#pragma once

#include <vector>
#include <SDL3/SDL_vulkan.h>

namespace rendererutils
{
    struct Extensions
    {
        uint32_t extensionCount = 0;
        std::vector<const char*> extensions;

        explicit operator std::vector<const char*>() const
        {
            return extensions;
        }

        const char* const* data() const
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
        for (auto extensions : sdlExtensions)
        {
            exts.extensions.push_back(extensions);
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
}