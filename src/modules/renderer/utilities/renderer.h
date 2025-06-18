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
    static uint32_t GetMinImageCount(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
    {
        VkSurfaceCapabilitiesKHR surfaceCapabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities);
        return surfaceCapabilities.minImageCount;
    }

    /// @note Queries the surface's capabilities to retrieve it's most suitable surface format
    static VkSurfaceFormatKHR ChooseBestSurfaceFormat(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
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

    static VkRenderingInfoKHR CreateRenderingInfo(
        VkRect2D renderArea,
        const std::vector<VkRenderingAttachmentInfoKHR>& colorAttachments,
        const VkRenderingAttachmentInfoKHR& depthAttachment = {},
        const VkRenderingAttachmentInfoKHR& stencilAttachment = {})
    {
        VkRenderingInfoKHR renderingInfo = {};
        renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
        renderingInfo.pNext = nullptr;
        renderingInfo.layerCount = 1;
        renderingInfo.viewMask = 0;
        renderingInfo.renderArea = renderArea;

        renderingInfo.colorAttachmentCount = static_cast<uint32_t>(colorAttachments.size());
        renderingInfo.pColorAttachments = colorAttachments.empty() ? nullptr : colorAttachments.data();

        if (!depthAttachment.imageView) {
            renderingInfo.pDepthAttachment = nullptr;
        } else {
            renderingInfo.pDepthAttachment = &depthAttachment;
        }

        if (!stencilAttachment.imageView) {
            renderingInfo.pStencilAttachment = nullptr;
        } else {
            renderingInfo.pStencilAttachment = &stencilAttachment;
        }

        return renderingInfo;
    }

    static VkRenderingAttachmentInfoKHR CreateRenderAttachment(VkImageView imageView,
        VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        VkImageLayout layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
    {
        VkRenderingAttachmentInfoKHR renderAttachment = {};
        renderAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
        renderAttachment.imageView = imageView;
        renderAttachment.imageLayout = layout;
        renderAttachment.loadOp = loadOp;
        renderAttachment.storeOp = storeOp;
        renderAttachment.clearValue = {};
        renderAttachment.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        renderAttachment.resolveMode = VK_RESOLVE_MODE_NONE;
        renderAttachment.resolveImageView = VK_NULL_HANDLE;
        renderAttachment.pNext = nullptr;
        return renderAttachment;
    }

    /// @note Queries the surface's capabilities to retrieve the best present mode
    static VkPresentModeKHR ChooseBestPresentMode(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
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
    static VkExtent2D ChooseBestExtent(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, const uint32_t width, const uint32_t height)
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
    static void SubmitOneTimeCommand(VkDevice device, VkCommandPool commandPool, VkQueue queue, const std::function<void(VkCommandBuffer)>& recordFunction)
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