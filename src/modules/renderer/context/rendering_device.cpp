//
// Created by lepag on 6/7/2025.
//

#include "rendering_device.h"

#include <set>

RenderingDevice::RenderingDevice() = default;

RenderingDevice::~RenderingDevice()
{
    cleanup();
}

bool RenderingDevice::init()
{
    if (!createInstance()) return false;
    if (!setupDebugMessenger()) return false;
    if (!selectPhysicalDevice()) return false;
    if (!createLogicalDevice()) return false;
    if (!createAllocator()) return false;
    if (!createCommandPool()) return false;
    if (!createDeviceFamilies()) return false;
    if (!querySupportedColorFormats()) return false;
    if (!querySupportedDepthFormats()) return false;
    if (!querySupportedStencilFormats()) return false;
    return true;
}

void RenderingDevice::cleanup()
{
    waitIdle();
    m_maid.cleanup();
    volkFinalize();
}

VkFormat RenderingDevice::queryColorFormat(VkFormat format)
{
    if (std::find(m_supportedColorFormats.begin(), m_supportedColorFormats.end(), format) != m_supportedColorFormats.end())
    {
        return format;
    }
    return m_supportedColorFormats[0];
}

VkFormat RenderingDevice::queryDepthFormat(VkFormat format)
{
    if (std::find(m_supportedDepthFormats.begin(), m_supportedDepthFormats.end(), format) != m_supportedDepthFormats.end())
    {
        return format;
    }
    return m_supportedDepthFormats[0];
}

VkFormat RenderingDevice::queryStencilFormat(VkFormat format)
{
    if (std::find(m_supportedStencilFormats.begin(), m_supportedStencilFormats.end(), format) != m_supportedStencilFormats.end())
    {
        return format;
    }
    return m_supportedStencilFormats[0];
}

VkSurfaceKHR RenderingDevice::createSurface(const Window *window) const
{
    SDL_Window* sdlWindow = window->getWindow();
    VkSurfaceKHR surface;
    if (!SDL_Vulkan_CreateSurface(sdlWindow, m_instance, nullptr, &surface))
    {
        Printer::error(SDL_GetError());
        return VK_NULL_HANDLE;
    }
    return surface;
}

DeviceQueue RenderingDevice::getPresentQueue(VkSurfaceKHR surface)
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
            presentQueue.type = DeviceQueueType::Present;
            presentQueue.family = i;

            vkGetDeviceQueue(m_logicalDevice, i, 0, &presentQueue.queue);
            return presentQueue;
        }
    }
    return {};
}

bool RenderingDevice::createInstance()
{
    volkInitialize();

    std::vector<const char*> extensions = {};
    uint32_t extensionCount = 0;
    char const* const* sdlExtensions = SDL_Vulkan_GetInstanceExtensions(&extensionCount);
    if (sdlExtensions)
    {
        extensions.reserve(extensionCount);
        for (uint32_t i = 0; i < extensionCount; ++i)
        {
            extensions.push_back(sdlExtensions[i]);
        }
    }
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    std::vector<const char*> validationLayers = {};
    validationLayers.push_back("VK_LAYER_KHRONOS_validation");

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Gyro";
    appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "GyroEngine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;
    appInfo.pNext = nullptr;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    createInfo.enabledExtensionCount = extensions.size();
    createInfo.ppEnabledExtensionNames = extensions.data();

    createInfo.enabledLayerCount = validationLayers.size();
    createInfo.ppEnabledLayerNames = validationLayers.data();

    createInfo.pNext = nullptr;

    VkResult result = vkCreateInstance(&createInfo, nullptr, &m_instance);
    if (result != VK_SUCCESS)
    {
        return false;
    }

    volkLoadInstance(m_instance);

    m_maid.add([&]()
    {
       destroyInstance();
    });

    return true;
}

bool RenderingDevice::setupDebugMessenger()
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
    createInfo.pfnUserCallback = &debugMessengerCallback;

    VkResult result = vkCreateDebugUtilsMessengerEXT(
        m_instance, &createInfo, nullptr, &m_debugMessenger);
    if (result != VK_SUCCESS)
    {
        Printer::error("Failed to create debug messenger: " + std::to_string(result));
        return false;
    }

    m_maid.add([&]()
    {
       destroyDebugMessenger();
    });

    return true;
}

bool RenderingDevice::selectPhysicalDevice()
{
    uint32_t physicalDeviceCount = 0;
    vkEnumeratePhysicalDevices(m_instance, &physicalDeviceCount, nullptr);

    std::vector<VkPhysicalDevice> foundPhysicalDevices(physicalDeviceCount);
    vkEnumeratePhysicalDevices(m_instance, &physicalDeviceCount, foundPhysicalDevices.data());

    if (physicalDeviceCount == 0)
    {
        Printer::error("No physical devices to pick from");
        return false;
    }

    std::map<uint32_t, VkPhysicalDevice> physicalDevices;
    for (uint32_t i = 0; i < physicalDeviceCount; i++)
    {
        uint32_t rank = 0;

        VkPhysicalDeviceProperties properties;
        VkPhysicalDeviceFeatures features;

        vkGetPhysicalDeviceProperties(foundPhysicalDevices[i], &properties);
        vkGetPhysicalDeviceFeatures(foundPhysicalDevices[i], &features);

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
            Printer::error("Unknown or unsupported GPU type enumerated");
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
        } else if (m_requiresTesselation && !features.tessellationShader)
        {
            rank = 0;
        }

        // Check one more time if this device shouldn't be supported
        if (rank == 0)
        {
            continue;
        }

        physicalDevices[i] = foundPhysicalDevices[i];
    }

    if (physicalDevices.empty())
    {
        Printer::error("No supported physical devices found");
        return false;
    }

    // Our physicalDevices is sorted in a map, first index will have the highest ranking
    m_physicalDevice = physicalDevices.rbegin()->second;
    return true;
}

bool RenderingDevice::createLogicalDevice()
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
        if ((queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT) &&
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
        if ((queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT) &&
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

    rendererutils::Extensions deviceExtensions;
    deviceExtensions = rendererutils::createExtensions({
        VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
        VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME,
        VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME,
        VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
        VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    });

    VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamicRenderingFeatures{};
    dynamicRenderingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR;
    dynamicRenderingFeatures.dynamicRendering = VK_TRUE;

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

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.enabledExtensionCount = deviceExtensions.extensionCount;
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();
    createInfo.pNext = &synchronization2Features;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    VkResult result = vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_logicalDevice);
    if (result != VK_SUCCESS)
    {
        Printer::error("Failed to create logical device");
        return false;
    }

    m_maid.add([&]()
    {
       destroyLogicalDevice();
    });

    return true;
}

bool RenderingDevice::createDeviceFamilies()
{
    uint32_t familyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &familyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(familyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &familyCount, queueFamilies.data());

    for (uint32_t i = 0; i < familyCount; i++)
    {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT && !m_deviceFamilies.getGraphicsQueue().isValid())
        {
            DeviceQueue queue{};
            queue.type = DeviceQueueType::Graphics;
            queue.family = i;

            vkGetDeviceQueue(m_logicalDevice, i, 0, &queue.queue);
            m_deviceFamilies.queues.push_back(queue);
        }

        if ((queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT) &&
        !(queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
        !(queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT))
            {
            DeviceQueue queue{};
            queue.type = DeviceQueueType::Transfer;
            queue.family = i;

            vkGetDeviceQueue(m_logicalDevice, i, 0, &queue.queue);
            m_deviceFamilies.queues.push_back(queue);
        }

        if (queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT && !m_deviceFamilies.getComputeQueue().isValid())
        {
            DeviceQueue queue{};
            queue.type = DeviceQueueType::Compute;
            queue.family = i;

            vkGetDeviceQueue(m_logicalDevice, i, 0, &queue.queue);
            m_deviceFamilies.queues.push_back(queue);
        }
    }

    if (m_deviceFamilies.queues.empty())
    {
        Printer::error("No device queue familes found");
        return false;
    }

    if (!m_deviceFamilies.getGraphicsQueue().isValid())
    {
        Printer::error("No graphics queue family found");
        return false;
    }

    return true;
}

bool RenderingDevice::createAllocator()
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
        Printer::error("Failed to create VMA allocator");
        return false;
    }

    m_maid.add([&]() {
        if (m_allocator != VK_NULL_HANDLE) {
            vmaDestroyAllocator(m_allocator);
            m_allocator = VK_NULL_HANDLE;
        }
    });

    return true;
}

bool RenderingDevice::createCommandPool()
{
    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = m_deviceFamilies.getGraphicsQueue().family;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    if (vkCreateCommandPool(m_logicalDevice, &poolInfo, nullptr, &m_commandPool) != VK_SUCCESS)
    {
        Printer::error("Failed to create command pool");
        return false;
    }
    m_maid.add([&]()
    {
       destroyCommandPool();
    });
    return true;
}

bool RenderingDevice::querySupportedColorFormats()
{
    std::vector availableColorFormats = {
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
        Printer::error("No supported color formats found");
        return false;
    }
    return true;
}

bool RenderingDevice::querySupportedDepthFormats()
{
    std::vector availableDepthFormats = {
        VK_FORMAT_D16_UNORM,
        VK_FORMAT_X8_D24_UNORM_PACK32,
        VK_FORMAT_D24_UNORM_S8_UINT,
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_S8_UINT
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
        Printer::error("No supported depth formats found");
        return false;
    }
    return true;
}

bool RenderingDevice::querySupportedStencilFormats()
{
    std::vector availableStencilFormats = {
        VK_FORMAT_D24_UNORM_S8_UINT,
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_S8_UINT
    };

    for (const auto& format : availableStencilFormats)
    {
        VkFormatProperties properties;
        vkGetPhysicalDeviceFormatProperties(m_physicalDevice, format, &properties);

        if (properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
        {
            m_supportedStencilFormats.push_back(format);
        }
    }

    if (m_supportedStencilFormats.empty())
    {
        Printer::error("No supported stencil formats found");
        return false;
    }
    return true;
}

void RenderingDevice::destroyCommandPool()
{
    if (m_commandPool != VK_NULL_HANDLE)
    {
        vkDestroyCommandPool(m_logicalDevice, m_commandPool, nullptr);
        m_commandPool = VK_NULL_HANDLE;
    }
}

void RenderingDevice::destroyAllocator()
{
    if (m_allocator != VK_NULL_HANDLE)
    {
        vmaDestroyAllocator(m_allocator);
        m_allocator = VK_NULL_HANDLE;
    }
}

void RenderingDevice::destroyLogicalDevice()
{
    if (m_logicalDevice != VK_NULL_HANDLE)
    {
        vkDestroyDevice(m_logicalDevice, nullptr);
        m_logicalDevice = VK_NULL_HANDLE;
    }
}

void RenderingDevice::destroyPhysicalDevice()
{
    if (m_physicalDevice != VK_NULL_HANDLE)
    {
        m_physicalDevice = VK_NULL_HANDLE;
    }
}

void RenderingDevice::destroyDebugMessenger()
{
    if (m_debugMessenger != VK_NULL_HANDLE)
    {
        vkDestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
        m_debugMessenger = VK_NULL_HANDLE;
    }
}

void RenderingDevice::destroyInstance()
{
    if (m_instance != VK_NULL_HANDLE)
    {
        vkDestroyInstance(m_instance, nullptr);
        m_instance = VK_NULL_HANDLE;
    }
}

VkBool32 RenderingDevice::debugMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData)
{
    Printer::print("[VULKAN]: " + std::string(pCallbackData->pMessage));
    return VK_FALSE;
}