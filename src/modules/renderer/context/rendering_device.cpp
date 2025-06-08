//
// Created by lepag on 6/7/2025.
//

#include "rendering_device.h"

RenderingDevice::RenderingDevice()
{}

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
    if (!createDeviceFamilies()) return false;
    return true;
}

void RenderingDevice::cleanup()
{
    m_maid.cleanup();
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
    auto extensions = rendererutils::getSDLExtensions();
    extensions.extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    extensions.extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    auto layers = rendererutils::createExtensions(
        {"VK_LAYER_KHRONOS_validation"}
        );

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Gyro";
    appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "GyroEngine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    createInfo.enabledExtensionCount = extensions.extensionCount;
    createInfo.ppEnabledExtensionNames = extensions.data();

    createInfo.enabledLayerCount = layers.extensionCount;
    createInfo.ppEnabledLayerNames = layers.data();

    VkResult result = vkCreateInstance(&createInfo, nullptr, &m_instance);
    if (result != VK_SUCCESS)
    {
        return false;
    }

    m_maid.add([&]()
    {
       destroyInstance();
    });

    volkLoadInstance(m_instance);
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
    rendererutils::Extensions deviceExtensions;
    deviceExtensions = rendererutils::createExtensions({
        VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
        VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME,
        VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME,
        VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
        VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME
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
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            DeviceQueue queue{};
            queue.type = DeviceQueueType::Graphics;
            queue.family = i;

            vkGetDeviceQueue(m_logicalDevice, i, 0, &queue.queue);
            m_deviceFamilies.queues.push_back(queue);
        }

        if (queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT)
        {
            DeviceQueue queue{};
            queue.type = DeviceQueueType::Transfer;
            queue.family = i;

            vkGetDeviceQueue(m_logicalDevice, i, 0, &queue.queue);
            m_deviceFamilies.queues.push_back(queue);
        }

        if (queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
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
