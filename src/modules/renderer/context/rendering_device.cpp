//
// Created by lepag on 6/7/2025.
//

#include "rendering_device.h"

#include <set>

namespace GyroEngine::Device
{
    RenderingDevice::RenderingDevice() = default;

RenderingDevice::~RenderingDevice()
{
    Cleanup();
}

bool RenderingDevice::Init()
{
    // Check for vulkan support
    bool vulkanSupported = SDL_Vulkan_LoadLibrary(nullptr) == 0;
    if (!vulkanSupported)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
            "VULKAN SUPPORT MISSING",
            "This system's hardware does not support Vulkan. Please ensure you have the latest drivers installed.",
            nullptr);
        throw std::runtime_error("Vulkan support is missing.");
    }

    if (!CreateInstance()) return false;
#ifdef DEBUG
    if (!SetupDebugMessenger()) return false;
#endif
    if (!SelectPhysicalDevice()) return false;
    if (!CreateLogicalDevice()) return false;
    if (!CreateAllocator()) return false;
    if (!CreateCommandPool()) return false;
    if (!CreateDeviceFamilies()) return false;
    if (!QueryAllSupportedColorFormats()) return false;
    if (!QueryAllSupportedDepthFormats()) return false;
    if (!FindPreferredColorFormat()) return false;
    if (!FindBestDepthFormat()) return false;
    return true;
}

void RenderingDevice::Cleanup()
{
    WaitForIdle();

    m_maid.Cleanup();
    volkFinalize();
}

void RenderingDevice::SetColorPreference(const PreferredColorFormatType preferredColorFormat)
{
    const PreferredColorFormatType oldPreferredColor = m_preferredColorType;
    VkFormat oldFormat = m_colorFormat;
    m_preferredColorType = preferredColorFormat;
    if (!FindPreferredColorFormat())
    {
        Logger::LogWarning("Fallback override, reusing previous format");
        m_colorFormat = oldFormat;
        m_preferredColorType = oldPreferredColor;
    }
}

void RenderingDevice::SetSwapchainColorFormat(VkFormat swapchainColorFormat)
{
    m_swapchainColorFormat = swapchainColorFormat;
}

VkFormat RenderingDevice::QueryForSupportedColorFormat(VkFormat format)
{
    if (std::find(m_supportedColorFormats.begin(), m_supportedColorFormats.end(), format) != m_supportedColorFormats.end())
    {
        return format;
    }
    return m_supportedColorFormats[0];
}

VkFormat RenderingDevice::QueryForSupportedDepthFormat(VkFormat format)
{
    if (std::find(m_supportedDepthFormats.begin(), m_supportedDepthFormats.end(), format) != m_supportedDepthFormats.end())
    {
        return format;
    }
    return m_supportedDepthFormats[0];
}

VkSurfaceKHR RenderingDevice::CreateSurfaceFromWindow(const Window *window) const
{
    SDL_Window* sdlWindow = window->GetWindowHandle();
    VkSurfaceKHR surface;
    if (!SDL_Vulkan_CreateSurface(sdlWindow, m_instance, nullptr, &surface))
    {
        Logger::LogError(SDL_GetError());
        return VK_NULL_HANDLE;
    }
    return surface;
}

DeviceQueue RenderingDevice::GetPresentQueueFromSurface(VkSurfaceKHR surface) const
{
    uint32_t familyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &familyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(familyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &familyCount, queueFamilies.data());

    for (uint32_t i = 0; i < familyCount; i++)
    {
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(m_physicalDevice, i, surface, &presentSupport);
        if (presentSupport)
        {
            DeviceQueue presentQueue;
            presentQueue.type = Utils::Device::QueueType::Present;
            presentQueue.family = i;

            vkGetDeviceQueue(m_logicalDevice, i, 0, &presentQueue.queue);
            return presentQueue;
        }
    }
    return {};
}

bool RenderingDevice::CreateInstance()
{
    volkInitialize();

    auto [extensionCount, extensions] = Utils::Device::GetSDLExtensions();
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    std::vector<const char*> validationLayers = {};
#ifdef DEBUG
    validationLayers.push_back("VK_LAYER_KHRONOS_validation");
#endif

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Gyro";
    appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "GyroEngine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;
    appInfo.pNext = nullptr;

    const std::vector<const char*> supportedInstanceExtensions = Utils::Device::enumerateVectorForSupportedInstanceExtensions(
        extensions);

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    createInfo.enabledExtensionCount = static_cast<uint32_t>(supportedInstanceExtensions.size());
    createInfo.ppEnabledExtensionNames = supportedInstanceExtensions.data();

    createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    createInfo.ppEnabledLayerNames = validationLayers.data();

    createInfo.pNext = nullptr;

    VkResult result = vkCreateInstance(&createInfo, nullptr, &m_instance);
    if (result != VK_SUCCESS)
    {
        return false;
    }

    volkLoadInstance(m_instance);

    m_maid.Add([&]
    {
       DestroyInstance();
    });

    return true;
}

bool RenderingDevice::SetupDebugMessenger()
{
    VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = &DebugMessengerCallback;

    VkResult result = vkCreateDebugUtilsMessengerEXT(
        m_instance, &createInfo, nullptr, &m_debugMessenger);
    if (result != VK_SUCCESS)
    {
        Logger::LogError("Failed to create debug messenger: " + std::to_string(result));
        return false;
    }

    m_maid.Add([&]
    {
       DestroyDebugMessenger();
    });

    return true;
}

bool RenderingDevice::SelectPhysicalDevice()
{
    uint32_t physicalDeviceCount = 0;
    vkEnumeratePhysicalDevices(m_instance, &physicalDeviceCount, nullptr);

    std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
    vkEnumeratePhysicalDevices(m_instance, &physicalDeviceCount, physicalDevices.data());

    if (physicalDeviceCount == 0)
    {
        Logger::LogError("No physical devices to pick from");
        return false;
    }

    std::vector<Utils::Device::RankedDevice> rankedDevices;

    for (uint32_t i = 0; i < physicalDeviceCount; i++)
    {
        uint32_t rank = 0;

        VkPhysicalDeviceProperties properties;
        VkPhysicalDeviceFeatures features;

        vkGetPhysicalDeviceProperties(physicalDevices[i], &properties);
        vkGetPhysicalDeviceFeatures(physicalDevices[i], &features);

        switch (properties.deviceType)
        {
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
            if (m_acceptDiscrete)
            {
                rank += 1000;
            }
            break;
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
            if (m_acceptIntegrated)
            {
                rank += 1000;
            }
            break;
        case VK_PHYSICAL_DEVICE_TYPE_CPU:
            if (m_acceptCPU)
            {
                rank += 1000;
            }
            break;
        default:
            Logger::LogError("Unknown or unsupported GPU type enumerated");
            break;
        }

        // Skip this device if it uses a device type we don't support
        if (rank == 0)
        {
            continue;
        }

        // Devices that support larger image dimensions get more priority
        rank += properties.limits.maxImageDimension2D;

        if (m_requiresTesselation && features.tessellationShader)
        {
            rank += 500;
        }
        else if (m_requiresTesselation && !features.tessellationShader)
        {
            rank = 0;
        }

        // Check one more time if this device shouldn't be supported
        if (rank == 0)
        {
            continue;
        }

        rankedDevices.push_back({physicalDevices[i], rank, properties});
    }

    if (rankedDevices.empty())
    {
        Logger::LogError("No supported physical devices found");
        return false;
    }

    // Pick the device with the highest rank
    const auto best = std::max_element(rankedDevices.begin(), rankedDevices.end(),
        [](const Utils::Device::RankedDevice& a, const Utils::Device::RankedDevice& b) {
            return a.score < b.score;
        });

    m_physicalDevice = best->physicalDevice;
    Logger::Log(std::string("Selected device: ") + best->properties.deviceName + " (score: " + std::to_string(best->score) + ")");
    return true;
}

bool RenderingDevice::CreateLogicalDevice()
{
    uint32_t familyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &familyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(familyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &familyCount, queueFamilies.data());

    int graphicsFamily = -1, computeFamily = -1, transferFamily = -1;

    // Find graphics family
    for (uint32_t i = 0; i < familyCount; ++i) {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            graphicsFamily = i;
            break;
        }
    }
    // Find dedicated compute family
    for (uint32_t i = 0; i < familyCount; ++i) {
        if (queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT &&
            !(queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
            computeFamily = i;
            break;
        }
    }
    // Fallback: any compute
    if (computeFamily == -1) {
        for (uint32_t i = 0; i < familyCount; ++i) {
            if (queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
                computeFamily = i;
                break;
            }
        }
    }
    // Find dedicated transfer family
    for (uint32_t i = 0; i < familyCount; ++i) {
        if (queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT &&
            !(queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
            !(queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT)) {
            transferFamily = i;
            break;
        }
    }
    // Fallback: any transfer
    if (transferFamily == -1) {
        for (uint32_t i = 0; i < familyCount; ++i) {
            if (queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT) {
                transferFamily = i;
                break;
            }
        }
    }

    std::set<uint32_t> uniqueFamilies;
    if (graphicsFamily != -1) uniqueFamilies.insert(graphicsFamily);
    if (computeFamily != -1) uniqueFamilies.insert(computeFamily);
    if (transferFamily != -1) uniqueFamilies.insert(transferFamily);

    float queuePriority = 1.0f;
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    for (uint32_t family : uniqueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = family;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    Utils::Device::Extensions deviceExtensions;
    deviceExtensions = Utils::Device::CreateExtensions({
        VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
        VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME,
        VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME,
        VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
        VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_EXT_DYNAMIC_RENDERING_UNUSED_ATTACHMENTS_EXTENSION_NAME
    });

    VkPhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT unusedAttachmentFeatures{};
    unusedAttachmentFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_UNUSED_ATTACHMENTS_FEATURES_EXT;
    unusedAttachmentFeatures.dynamicRenderingUnusedAttachments = VK_TRUE;

    VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamicRenderingFeatures{};
    dynamicRenderingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR;
    dynamicRenderingFeatures.dynamicRendering = VK_TRUE;
    dynamicRenderingFeatures.pNext = &unusedAttachmentFeatures;

    VkPhysicalDeviceDescriptorIndexingFeatures indexingFeatures{};
    indexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
    indexingFeatures.descriptorBindingPartiallyBound = VK_TRUE;
    indexingFeatures.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
    indexingFeatures.descriptorBindingStorageImageUpdateAfterBind = VK_TRUE;
    indexingFeatures.descriptorBindingStorageBufferUpdateAfterBind = VK_TRUE;
    indexingFeatures.descriptorBindingUniformBufferUpdateAfterBind = VK_TRUE;
    indexingFeatures.descriptorBindingVariableDescriptorCount = VK_TRUE;
    indexingFeatures.runtimeDescriptorArray = VK_TRUE;
    indexingFeatures.pNext = &dynamicRenderingFeatures;

    VkPhysicalDeviceSynchronization2FeaturesKHR synchronization2Features{};
    synchronization2Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES_KHR;
    synchronization2Features.pNext = &indexingFeatures;

    const std::vector<const char*> supportedDeviceExtensions = Utils::Device::EnumerateVectorForSupportedDeviceExtensions(
        m_physicalDevice, deviceExtensions.extensions);

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(supportedDeviceExtensions.size());
    createInfo.ppEnabledExtensionNames = supportedDeviceExtensions.data();
    createInfo.pNext = &synchronization2Features;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    VkResult result = vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_logicalDevice);
    if (result != VK_SUCCESS)
    {
        Logger::LogError("Failed to create logical device");
        return false;
    }

    m_maid.Add([&]
    {
       DestroyLogicalDevice();
    });

    return true;
}

bool RenderingDevice::CreateDeviceFamilies()
{
    uint32_t familyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &familyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(familyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &familyCount, queueFamilies.data());

    for (uint32_t i = 0; i < familyCount; i++)
    {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT && !m_deviceFamilies.GetGraphicsQueue().isValid())
        {
            DeviceQueue queue{};
            queue.type = Utils::Device::QueueType::Graphics;
            queue.family = i;

            vkGetDeviceQueue(m_logicalDevice, i, 0, &queue.queue);
            m_deviceFamilies.queues.push_back(queue);
        }

        if (queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT &&
        !(queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
        !(queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT))
            {
            DeviceQueue queue{};
            queue.type = Utils::Device::QueueType::Transfer;
            queue.family = i;

            vkGetDeviceQueue(m_logicalDevice, i, 0, &queue.queue);
            m_deviceFamilies.queues.push_back(queue);
        }

        if (queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT && !m_deviceFamilies.GetComputeQueue().isValid())
        {
            DeviceQueue queue{};
            queue.type = Utils::Device::QueueType::Compute;
            queue.family = i;

            vkGetDeviceQueue(m_logicalDevice, i, 0, &queue.queue);
            m_deviceFamilies.queues.push_back(queue);
        }
    }

    if (m_deviceFamilies.queues.empty())
    {
        Logger::LogError("No device queue familes found");
        return false;
    }

    if (!m_deviceFamilies.GetGraphicsQueue().isValid())
    {
        Logger::LogError("No graphics queue family found");
        return false;
    }

    return true;
}

bool RenderingDevice::CreateAllocator()
{
    VmaVulkanFunctions vmaFuncs = {};
    vmaFuncs.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
    vmaFuncs.vkGetDeviceProcAddr = vkGetDeviceProcAddr;

    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = m_physicalDevice;
    allocatorInfo.device = m_logicalDevice;
    allocatorInfo.instance = m_instance;
    allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_3;
    allocatorInfo.pVulkanFunctions = &vmaFuncs;

    if (vmaCreateAllocator(&allocatorInfo, &m_allocator) != VK_SUCCESS) {
        Logger::LogError("Failed to create VMA allocator");
        return false;
    }

    m_maid.Add([&] {
        DestroyAllocator();
    });

    return true;
}

bool RenderingDevice::CreateCommandPool()
{
    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = m_deviceFamilies.GetGraphicsQueue().family;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    if (vkCreateCommandPool(m_logicalDevice, &poolInfo, nullptr, &m_commandPool) != VK_SUCCESS)
    {
        Logger::LogError("Failed to create command pool");
        return false;
    }
    m_maid.Add([&]
    {
       DestroyCommandPool();
    });
    return true;
}

bool RenderingDevice::QueryAllSupportedColorFormats()
{
    const std::vector availableColorFormats = {
        VK_FORMAT_R8G8B8A8_UNORM,
        VK_FORMAT_B8G8R8A8_UNORM,
        VK_FORMAT_R8G8B8A8_SRGB,
        VK_FORMAT_B8G8R8A8_SRGB,
        VK_FORMAT_A2B10G10R10_UNORM_PACK32,
        VK_FORMAT_R16G16B16A16_SFLOAT,
        VK_FORMAT_R32G32B32A32_SFLOAT,
    };

    for (const auto& format : availableColorFormats)
    {
        VkFormatProperties properties;
        vkGetPhysicalDeviceFormatProperties(m_physicalDevice, format, &properties);

        if (properties.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT)
        {
            m_supportedColorFormats.push_back(format);
        }
    }

    if (m_supportedColorFormats.empty())
    {
        Logger::LogError("No supported color formats found");
        return false;
    }
    return true;
}

bool RenderingDevice::QueryAllSupportedDepthFormats()
{
    const std::vector availableDepthFormats = {
        VK_FORMAT_D24_UNORM_S8_UINT,
        VK_FORMAT_D32_SFLOAT_S8_UINT
    };

    for (const auto& format : availableDepthFormats)
    {
        VkFormatProperties properties;
        vkGetPhysicalDeviceFormatProperties(m_physicalDevice, format, &properties);

        if (properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
        {
            m_supportedDepthFormats.push_back(format);
        }
    }

    if (m_supportedDepthFormats.empty())
    {
        Logger::LogError("No supported depth formats found");
        return false;
    }
    return true;
}
void RenderingDevice::DestroyCommandPool()
{
    if (m_commandPool != VK_NULL_HANDLE)
    {
        vkDestroyCommandPool(m_logicalDevice, m_commandPool, nullptr);
        m_commandPool = VK_NULL_HANDLE;
    }
}

void RenderingDevice::DestroyAllocator()
{
    if (m_allocator != VK_NULL_HANDLE)
    {
        vmaDestroyAllocator(m_allocator);
        m_allocator = VK_NULL_HANDLE;
    }
}

void RenderingDevice::DestroyLogicalDevice()
{
    if (m_logicalDevice != VK_NULL_HANDLE)
    {
        vkDestroyDevice(m_logicalDevice, nullptr);
        m_logicalDevice = VK_NULL_HANDLE;
    }
}

void RenderingDevice::DestroyPhysicalDevice()
{
    if (m_physicalDevice != VK_NULL_HANDLE)
    {
        m_physicalDevice = VK_NULL_HANDLE;
    }
}

void RenderingDevice::DestroyDebugMessenger()
{
    if (m_debugMessenger != VK_NULL_HANDLE)
    {
        vkDestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
        m_debugMessenger = VK_NULL_HANDLE;
    }
}

void RenderingDevice::DestroyInstance()
{
    if (m_instance != VK_NULL_HANDLE)
    {
        vkDestroyInstance(m_instance, nullptr);
        m_instance = VK_NULL_HANDLE;
    }
}

bool RenderingDevice::FindPreferredColorFormat()
{
    std::string typeString = "none";
    for (const auto& format : m_supportedColorFormats)
    {
        switch (m_preferredColorType)
        {
        case PreferredColorFormatType::sRGB:
            typeString = "sRGB";
            if (format == VK_FORMAT_R8G8B8A8_SRGB || format == VK_FORMAT_B8G8R8A8_SRGB)
            {
                m_colorFormat = format;
                return true;
            }
            break;
        case PreferredColorFormatType::HDR:
            typeString = "HDR";
            if (format == VK_FORMAT_R16G16B16A16_SFLOAT || format == VK_FORMAT_R32G32B32A32_SFLOAT)
            {
                m_colorFormat = format;
                return true;
            }
            break;
        case PreferredColorFormatType::LINEAR:
            typeString = "linear";
            if (format == VK_FORMAT_A2B10G10R10_UNORM_PACK32)
            {
                m_colorFormat = format;
                return true;
            }
            break;
        case PreferredColorFormatType::COMPATIBLE:
            typeString = "compatible";
            // Select the first format that is compatible
            m_colorFormat = format;
            return true;
        case PreferredColorFormatType::DEFAULT:
            typeString = "default";
            // Fallback to the first supported format
            m_colorFormat = m_supportedColorFormats.front();
            return true;
        default: break;
        }
    }

    m_colorFormat = m_supportedColorFormats.empty() ? VK_FORMAT_UNDEFINED : m_supportedColorFormats.front();
    Logger::LogWarning("Attempt to find a suitable color format for type " + typeString + " failed. Using fallback format: " + std::to_string(m_colorFormat));
    return m_colorFormat != VK_FORMAT_UNDEFINED;
}

bool RenderingDevice::FindBestDepthFormat()
{
    // Prioritize commonly used depth formats
    for (const auto& format : m_supportedDepthFormats) {
        if (format == VK_FORMAT_D32_SFLOAT || format == VK_FORMAT_D32_SFLOAT_S8_UINT) {
            m_depthFormat = format;
            m_stencilFormat = format;
            return true;
        }
    }

    // Fallback to the first supported format
    m_depthFormat = m_supportedDepthFormats.empty() ? VK_FORMAT_UNDEFINED : m_supportedDepthFormats.front();
    m_stencilFormat = m_depthFormat;
    return m_depthFormat != VK_FORMAT_UNDEFINED;
}

VkBool32 RenderingDevice::DebugMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                 VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                 void *pUserData)
{
    Logger::Log("[VULKAN]: " + std::string(pCallbackData->pMessage));
    return VK_FALSE;
}
}
