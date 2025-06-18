//
// Created by lepag on 6/8/2025.
//

#pragma once

#include <functional>
#include <stdexcept>
#include <vector>
#include <volk.h>

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

namespace GyroEngine::Utils::Renderer
{
    /// @note Queries the surface's capabilities to retrieve the minimum image count required
    static uint32_t getMinImageCount(const VkPhysicalDevice physicalDevice, const VkSurfaceKHR surface)
    {
        VkSurfaceCapabilitiesKHR surfaceCapabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities);
        return surfaceCapabilities.minImageCount;
    }

    /// @note Queries the surface's capabilities to retrieve it's most suitable surface format
    static VkSurfaceFormatKHR chooseBestSurfaceFormat(const VkPhysicalDevice physicalDevice, const VkSurfaceKHR surface)
    {
        uint32_t formatCount = 0;

        // Query for number of surface formats
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
        if (formatCount == 0) {
            throw std::runtime_error("No surface formats available");
        }

        // Populate available formats
        std::vector<VkSurfaceFormatKHR> availableFormats(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, availableFormats.data());

        // Find preferred format
        for (const auto& availableFormat : availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat; // Preferred format
            }
        }

        return availableFormats[0]; // Fallback to the first available format
    }

    static VkRenderingInfoKHR CreateRenderStruct(const VkExtent2D extent,
        const uint32_t colorCount = 1,
        uint32_t depthCount = 0)
    {
        VkRenderingInfoKHR renderingInfo = {};
        renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
        renderingInfo.flags = 0;
        renderingInfo.renderArea.offset = { 0, 0 };
        renderingInfo.renderArea.extent = extent;
        renderingInfo.colorAttachmentCount = colorCount;
        renderingInfo.pColorAttachments = nullptr; // Will be set later
        renderingInfo.pDepthAttachment = nullptr; // Will be set later
        renderingInfo.pStencilAttachment = nullptr; // Will be set later
        renderingInfo.layerCount = 1; // Default to 1 layer
        return renderingInfo;
    }

    static VkRenderingAttachmentInfoKHR CreateRenderAttachment(const VkImageView imageView,
        const VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        const VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        const VkImageLayout layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
    {
        VkRenderingAttachmentInfoKHR renderAttachment = {};
        renderAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
        renderAttachment.imageView = imageView;
        renderAttachment.imageLayout = layout;
        renderAttachment.loadOp = loadOp;
        renderAttachment.storeOp = storeOp;
        renderAttachment.clearValue.color.float32[0] = 0.0f;
        return renderAttachment;
    }

    /// @note Queries the surface's capabilities to retrieve the best present mode
    static VkPresentModeKHR ChooseBestPresentMode(const VkPhysicalDevice physicalDevice, const VkSurfaceKHR surface)
    {
        uint32_t presentCount = 0;

        // Query for number of present modes
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentCount, nullptr);
        if (presentCount == 0) {
            throw std::runtime_error("No present modes available");
        }

        // Populate available present modes
        std::vector<VkPresentModeKHR> availablePresentModes(presentCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentCount, availablePresentModes.data());

        // Find preferred present mode
        for (const auto& presentMode : availablePresentModes) {
            if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return presentMode; // Preferred mode
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR; // FIFO is always available and is a safe fallback
    }

    /// @note Queries the surface's capabilities to retrieve the best extent
    static VkExtent2D ChooseBestExtent(const VkPhysicalDevice physicalDevice, const VkSurfaceKHR surface, const uint32_t width, const uint32_t height)
    {
        // Query surface capabilities
        VkSurfaceCapabilitiesKHR surfaceCapabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities);

        // Check if the extent is dynamic
        // ^ A dynamic extent means the surface size can be resized separately from the window size
        if (surfaceCapabilities.currentExtent.width == UINT32_MAX) {
            VkExtent2D extent = { width, height };
            extent.width = std::max(surfaceCapabilities.minImageExtent.width, std::min(extent.width, surfaceCapabilities.maxImageExtent.width));
            extent.height = std::max(surfaceCapabilities.minImageExtent.height, std::min(extent.height, surfaceCapabilities.maxImageExtent.height));
            return extent;
        }

        // Extent is fixed, return the current extent
        // ^ A fixed extent means the surface size is determined by the window size
        return surfaceCapabilities.currentExtent;
    }

    /// @note Executes an action on a temporary command buffer
    static void SubmitOneTimeCommand(const VkDevice device, const VkCommandPool commandPool, const VkQueue queue, const std::function<void(VkCommandBuffer)>& recordFunction)
    {
        VkCommandBufferAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = commandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        recordFunction(commandBuffer);

        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(queue);

        vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
    }
}