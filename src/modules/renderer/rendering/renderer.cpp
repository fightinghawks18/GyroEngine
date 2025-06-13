//
// Created by lepag on 6/8/2025.
//

#include "renderer.h"

#include "context/rendering_device.h"

Renderer::~Renderer()
{
    cleanup();
}

bool Renderer::init(Window* window)
{
    m_window = window;
    m_surface = m_device.createSurface(m_window);
    m_presentQueue = m_device.getPresentQueue(m_surface).queue;

    if (!createSwapchain()) return false;
    if (!createSwapchainImages()) return false;
    if (!createImages()) return false;
    if (!createCommandBuffers()) return false;
    if (!createSyncObjects()) return false;
    return true;
}

void Renderer::cleanup()
{
    m_device.waitIdle();
    destroySwapchain();
    if (m_surface != VK_NULL_HANDLE)
    {
        vkDestroySurfaceKHR(m_device.getInstance(), m_surface, nullptr);
        m_surface = VK_NULL_HANDLE;
    }
    destroySwapchainImages();
    destroyImages();
    destroyCommandBuffers();
    destroySyncObjects();
}

bool Renderer::recreate()
{
    m_device.waitIdle();

    destroyCommandBuffers();
    destroySyncObjects();

    if (!createSwapchain()) return false;
    if (!createSwapchainImages()) return false;

    createCommandBuffers();
    createSyncObjects();

    Printer::print("Recreated swapchain on frame " + std::to_string(m_currentFrame));
    return true;
}

void Renderer::advanceFrame()
{
    m_currentFrame = (m_currentFrame + 1) % m_device.getMaxFramesInFlight();
}

VkCommandBuffer Renderer::beginFrame()
{
    return startRecord();
}

void Renderer::setViewport(const Viewport& viewport)
{
    m_viewport = viewport;
}

void Renderer::renderFrame()
{
    m_renderPipeline.execute();
}

void Renderer::endFrame()
{
    endRecord();
    submitRender();
    presentRender();
}

VkCommandBuffer Renderer::startRecord()
{
    if (m_needsRecreation)
    {
        if (!recreate())
        {
            Printer::error("Failed to recreate swapchain");
            return VK_NULL_HANDLE;
        }
        m_needsRecreation = false;
    }

    VkResult waitResult = vkWaitForFences(m_device.getLogicalDevice(), 1, &m_inFlightFences[m_currentFrame], VK_TRUE, 100000000);
    if (waitResult == VK_TIMEOUT)
    {
        Printer::error("Fence timed out on frame index " + std::to_string(m_currentFrame));
        return VK_NULL_HANDLE;
    } if (waitResult != VK_SUCCESS)
    {
        Printer::error("Failed to wait for fence: " + std::to_string(waitResult));
        return VK_NULL_HANDLE;
    }

    vkResetFences(m_device.getLogicalDevice(), 1, &m_inFlightFences[m_currentFrame]);

    VkResult imageAcquireResult = vkAcquireNextImageKHR(
        m_device.getLogicalDevice(),
        m_swapchain,
        UINT64_MAX,
        m_imageAvailableSemaphores[m_currentFrame],
        VK_NULL_HANDLE,
        &m_currentImageIndex
    );

    m_swapchainImages[m_currentImageIndex]->makeColor();

    if (imageAcquireResult == VK_ERROR_OUT_OF_DATE_KHR
        || imageAcquireResult == VK_SUBOPTIMAL_KHR)
    {
        m_needsRecreation = true;
        Printer::print("Image acquire out of date or suboptimal, recreating on frame " + std::to_string((m_currentFrame + 1) % m_device.getMaxFramesInFlight() ));
        return VK_NULL_HANDLE;
    } if (imageAcquireResult != VK_SUCCESS)
    {
        Printer::error("Failed to acquire swapchain image: " + std::to_string(imageAcquireResult));
        return VK_NULL_HANDLE;
    }

    VkCommandBuffer commandBuffer = m_commandBuffers[m_currentFrame];

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
    {
        Printer::error("Failed to begin command buffer recording");
        return VK_NULL_HANDLE;
    }

    if (m_viewport.width > 0 && m_viewport.height > 0)
    {
        VkViewport viewport{};
        viewport.x = m_viewport.x * static_cast<float>(m_swapchainExtent.width);
        viewport.y = m_viewport.y * static_cast<float>(m_swapchainExtent.height);
        viewport.width = m_viewport.width * static_cast<float>(m_swapchainExtent.width);
        viewport.height = m_viewport.height * static_cast<float>(m_swapchainExtent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = m_viewport.depth;

        VkRect2D scissor{};
        scissor.offset = { static_cast<int32_t>(viewport.x), static_cast<int32_t>(viewport.y) };
        scissor.extent = { static_cast<uint32_t>(viewport.width), static_cast<uint32_t>(viewport.height) };

        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    }

    return commandBuffer;
}

void Renderer::presentRender()
{
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &m_renderFinishedSemaphores[m_currentFrame];

    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &m_swapchain;
    presentInfo.pImageIndices = &m_currentImageIndex;

    VkResult result = vkQueuePresentKHR(m_presentQueue, &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR
        || result == VK_SUBOPTIMAL_KHR)
    {
        m_needsRecreation = true;
        Printer::print("Swapchain out of date or suboptimal, recreating on frame " + std::to_string(m_currentFrame));
    } else if (result != VK_SUCCESS)
    {
        Printer::error("Failed to present swapchain image: " + std::to_string(result));
    }

    advanceFrame();
}

void Renderer::submitRender()
{
    m_swapchainImages[m_currentImageIndex]->makePresent();

    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &m_imageAvailableSemaphores[m_currentFrame];
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_commandBuffers[m_currentFrame];

    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &m_renderFinishedSemaphores[m_currentFrame];

    if (vkQueueSubmit(m_presentQueue, 1, &submitInfo, m_inFlightFences[m_currentFrame]) != VK_SUCCESS)
    {
        Printer::error("Failed to submit command buffer");
    }
}

void Renderer::endRecord()
{
    VkCommandBuffer commandBuffer = m_commandBuffers[m_currentFrame];
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
    {
        Printer::error("Failed to end command buffer recording");
    }
}

bool Renderer::createSwapchain()
{
    // Query surface capabilities
    // ^ Ensures that moving to a different monitor with a different colorspace
    // ^ or requirements doesn't cause any problems
    m_surfaceFormat = rendererutils::chooseBestSurfaceFormat(m_device.getPhysicalDevice(), m_surface);
    m_presentMode = rendererutils::chooseBestPresentMode(m_device.getPhysicalDevice(), m_surface);
    m_swapchainImageFormat = m_surfaceFormat.format;
    m_swapchainExtent = rendererutils::chooseBestExtent(m_device.getPhysicalDevice(), m_surface, m_window->getWidth(), m_window->getHeight());

    uint32_t minImageCount = rendererutils::getMinImageCount(m_device.getPhysicalDevice(), m_surface);

    VkSwapchainCreateInfoKHR swapchainInfo{};
    swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainInfo.surface = m_surface;
    swapchainInfo.minImageCount = minImageCount;
    swapchainInfo.imageFormat = m_surfaceFormat.format;
    swapchainInfo.imageColorSpace = m_surfaceFormat.colorSpace;
    swapchainInfo.imageExtent = m_swapchainExtent;
    swapchainInfo.imageArrayLayers = 1;
    swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchainInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainInfo.presentMode = m_presentMode;
    swapchainInfo.clipped = VK_TRUE;
    swapchainInfo.oldSwapchain = m_swapchain;

    VkSwapchainKHR newSwapchain = VK_NULL_HANDLE;
    if (vkCreateSwapchainKHR(m_device.getLogicalDevice(), &swapchainInfo, nullptr, &newSwapchain) != VK_SUCCESS)
    {
        Printer::error("Failed to create swapchain");
        return false;
    }

    if (m_swapchain)
    {
        destroySwapchain();
        destroySwapchainImages();
    }

    m_swapchain = newSwapchain;

    return true;
}

bool Renderer::createSwapchainImages()
{
    uint32_t imageCount = 0;
    vkGetSwapchainImagesKHR(m_device.getLogicalDevice(), m_swapchain, &imageCount, nullptr);
    std::vector<VkImage> swapchainVkImages(imageCount);
    vkGetSwapchainImagesKHR(m_device.getLogicalDevice(), m_swapchain, &imageCount, swapchainVkImages.data());

    m_swapchainImages.reserve(imageCount);

    for (uint32_t i = 0; i < imageCount; ++i)
    {
        auto image = new Image(m_device);

        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = swapchainVkImages[i];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = m_swapchainImageFormat;
        viewInfo.components = {
            VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY
        };
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        VkImageView imageView = VK_NULL_HANDLE;

        if (vkCreateImageView(m_device.getLogicalDevice(), &viewInfo, nullptr, &imageView) != VK_SUCCESS)
        {
            Printer::error("Failed to create image view");
            return false;
        }

        image->setAspectMask(VK_IMAGE_ASPECT_COLOR_BIT);

        if (!image->init(swapchainVkImages[i], imageView))
        {
            Printer::error("Failed to initialize swapchain image");
            return false;
        }

        image->makePresent();
        m_swapchainImages.push_back(image);
    }
    return true;
}

bool Renderer::createImages()
{
    for (uint32_t i = 0; i < m_device.getMaxFramesInFlight(); ++i)
    {
        auto colorImage = new Image(m_device);
        auto depthImage = new Image(m_device);
        auto pipelineImage = new Image(m_device);

        colorImage->setAspectMask(VK_IMAGE_ASPECT_COLOR_BIT)
            .setUsage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
            .setExtent({m_swapchainExtent.width, m_swapchainExtent.height, 1})
            .setFormat(m_swapchainImageFormat);

        depthImage->setAspectMask(VK_IMAGE_ASPECT_DEPTH_BIT)
            .setUsage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
            .setExtent({m_swapchainExtent.width, m_swapchainExtent.height, 1})
            .setFormat(m_device.getPreferredDepthFormat());

        pipelineImage->setAspectMask(VK_IMAGE_ASPECT_COLOR_BIT)
            .setUsage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
            .setExtent({m_swapchainExtent.width, m_swapchainExtent.height, 1})
            .setFormat(m_swapchainImageFormat);

        if (!colorImage->init(VK_NULL_HANDLE, VK_NULL_HANDLE))
        {
            Printer::error("Failed to create color image");
            return false;
        }
        if (!depthImage->init(VK_NULL_HANDLE, VK_NULL_HANDLE))
        {
            Printer::error("Failed to create depth image");
            return false;
        }
        if (!pipelineImage->init(VK_NULL_HANDLE, VK_NULL_HANDLE))
        {
            Printer::error("Failed to create pipeline image");
            return false;
        }

        m_colorImages.push_back(colorImage);
        m_depthImages.push_back(depthImage);
        m_pipelineImages.push_back(pipelineImage);
    }
    return true;
}

bool Renderer::createCommandBuffers()
{
    m_commandBuffers.resize(m_device.getMaxFramesInFlight());

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_device.getCommandPool();
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size());

    if (vkAllocateCommandBuffers(m_device.getLogicalDevice(), &allocInfo, m_commandBuffers.data()) != VK_SUCCESS)
    {
        Printer::error("Failed to allocate command buffers");
        return false;
    }
    return true;
}

bool Renderer::createSyncObjects()
{
    m_imageAvailableSemaphores.resize(m_device.getMaxFramesInFlight());
    m_renderFinishedSemaphores.resize(m_device.getMaxFramesInFlight());
    m_inFlightFences.resize(m_device.getMaxFramesInFlight());

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < m_device.getMaxFramesInFlight(); ++i)
    {
        if (vkCreateSemaphore(m_device.getLogicalDevice(), &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(m_device.getLogicalDevice(), &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(m_device.getLogicalDevice(), &fenceInfo, nullptr, &m_inFlightFences[i]) != VK_SUCCESS)
        {
            Printer::error("Failed to create sync objects");
            return false;
        }
    }
    return true;
}

void Renderer::destroyCommandBuffers()
{
    for (auto& commandBuffer : m_commandBuffers)
    {
        if (commandBuffer != VK_NULL_HANDLE)
        {
            vkFreeCommandBuffers(m_device.getLogicalDevice(), m_device.getCommandPool(), 1, &commandBuffer);
            commandBuffer = VK_NULL_HANDLE;
        }
    }
    m_commandBuffers.clear();
}

void Renderer::destroySyncObjects()
{
    for (size_t i = 0; i < m_device.getMaxFramesInFlight(); ++i)
    {
        if (m_imageAvailableSemaphores[i] != VK_NULL_HANDLE)
        {
            vkDestroySemaphore(m_device.getLogicalDevice(), m_imageAvailableSemaphores[i], nullptr);
            m_imageAvailableSemaphores[i] = VK_NULL_HANDLE;
        }
        if (m_renderFinishedSemaphores[i] != VK_NULL_HANDLE)
        {
            vkDestroySemaphore(m_device.getLogicalDevice(), m_renderFinishedSemaphores[i], nullptr);
            m_renderFinishedSemaphores[i] = VK_NULL_HANDLE;
        }
        if (m_inFlightFences[i] != VK_NULL_HANDLE)
        {
            vkDestroyFence(m_device.getLogicalDevice(), m_inFlightFences[i], nullptr);
            m_inFlightFences[i] = VK_NULL_HANDLE;
        }
    }
}

void Renderer::destroySwapchainImages()
{
    for (auto& image : m_swapchainImages)
    {
        if (image)
        {
            image->cleanup();
            delete image;
            image = nullptr;
        }
    }
    m_swapchainImages.clear();
}

void Renderer::destroyImages()
{
    for (auto& image : m_colorImages)
    {
        if (image)
        {
            image->cleanup();
            delete image;
            image = nullptr;
        }
    }
    m_colorImages.clear();

    for (auto& image : m_depthImages)
    {
        if (image)
        {
            image->cleanup();
            delete image;
            image = nullptr;
        }
    }
    m_depthImages.clear();

    for (auto& image : m_pipelineImages)
    {
        if (image)
        {
            image->cleanup();
            delete image;
            image = nullptr;
        }
    }
    m_pipelineImages.clear();
}

void Renderer::destroySwapchain()
{
    if (m_swapchain != VK_NULL_HANDLE)
    {
        vkDestroySwapchainKHR(m_device.getLogicalDevice(), m_swapchain, nullptr);
        m_swapchain = VK_NULL_HANDLE;
    }
}
