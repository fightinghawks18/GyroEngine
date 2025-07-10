//
// Created by lepag on 6/8/2025.
//

#include "renderer.h"

#include "context/rendering_device.h"

namespace GyroEngine::Rendering
{
    Renderer::~Renderer()
{
    Cleanup();
}

bool Renderer::Init(Window* window)
{
    m_window = window;
    m_surface = m_device.CreateSurfaceFromWindow(m_window);
    m_presentQueue = m_device.GetPresentQueueFromSurface(m_surface).queue;

    if (!CreateSwapchain()) return false;
    if (!CreateSwapchainImages()) return false;
    if (!CreateImages()) return false;
    if (!CreateSampler()) return false;
    if (!CreateCommandBuffers()) return false;
    if (!CreateSyncObjects()) return false;
    return true;
}

void Renderer::Cleanup()
{
    m_device.WaitForIdle();
    DestroySwapchain();
    if (m_surface != VK_NULL_HANDLE)
    {
        vkDestroySurfaceKHR(m_device.GetInstance(), m_surface, nullptr);
        m_surface = VK_NULL_HANDLE;
    }
    DestroySwapchainImages();
    DestroyImages();
    DestroySampler();
    DestroyCommandBuffers();
    DestroySyncObjects();
}

bool Renderer::Resize()
{
    m_device.WaitForIdle();

    DestroyCommandBuffers();
    DestroySyncObjects();
    DestroyImages();

    if (!CreateSwapchain()) return false;
    if (!CreateSwapchainImages()) return false;
    if (!CreateImages()) return false;
    if (!CreateCommandBuffers()) return false;
    if (!CreateSyncObjects()) return false;

    Logger::Log("Recreated swapchain on frame " + std::to_string(m_currentFrame));
    return true;
}

void Renderer::NextFrameIndex()
{
    m_currentFrame = (m_currentFrame + 1) % m_device.GetMaxFramesInFlight();
}

bool Renderer::RecordFrame()
{
    return StartRecord();
}

void Renderer::BindViewport(const Viewport& viewport)
{
    m_viewport = viewport;
}

void Renderer::BindRenderingInfo(const VkRenderingInfoKHR &renderingInfo)
{
    m_renderingInfo = renderingInfo;
}

void Renderer::StartRender() const
{
    vkCmdBeginRenderingKHR(m_commandBuffers[m_currentFrame], &m_renderingInfo);
}

void Renderer::EndRender() const
{
    vkCmdEndRenderingKHR(m_commandBuffers[m_currentFrame]);
}

void Renderer::SubmitFrame()
{
    EndRecord();
    SubmitRender();
    PresentRender();
}

bool Renderer::StartRecord()
{
    if (m_needsRecreation)
    {
        if (!Resize())
        {
            Logger::LogError("Failed to recreate swapchain");
            return false;
        }
        m_needsRecreation = false;
    }

    VkResult waitResult = vkWaitForFences(m_device.GetLogicalDevice(), 1, &m_inFlightFences[m_currentFrame], VK_TRUE, 100000000);
    if (waitResult == VK_TIMEOUT)
    {
        Logger::LogError("Fence timed out on frame index " + std::to_string(m_currentFrame));
        return false;
    } if (waitResult != VK_SUCCESS)
    {
        Logger::LogError("Failed to wait for fence: " + std::to_string(waitResult));
        return false;
    }

    vkResetFences(m_device.GetLogicalDevice(), 1, &m_inFlightFences[m_currentFrame]);

    VkResult imageAcquireResult = vkAcquireNextImageKHR(
        m_device.GetLogicalDevice(),
        m_swapchain,
        UINT64_MAX,
        m_imageAvailableSemaphores[m_currentFrame],
        VK_NULL_HANDLE,
        &m_currentImageIndex
    );

    m_swapchainImages[m_currentImageIndex]->MakeColor();
    m_depthImages[m_currentFrame]->MakeDepthStencil();

    if (imageAcquireResult == VK_ERROR_OUT_OF_DATE_KHR
        || imageAcquireResult == VK_SUBOPTIMAL_KHR)
    {
        m_needsRecreation = true;
        Logger::Log("Image acquire out of date or suboptimal, recreating on frame " + std::to_string((m_currentFrame + 1) % m_device.GetMaxFramesInFlight() ));
        return false;
    } if (imageAcquireResult != VK_SUCCESS)
    {
        Logger::LogError("Failed to acquire swapchain image: " + std::to_string(imageAcquireResult));
        return false;
    }

    VkCommandBuffer commandBuffer = m_commandBuffers[m_currentFrame];

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
    {
        Logger::LogError("Failed to begin command buffer recording");
        return false;
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

    return true;
}

void Renderer::PresentRender()
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
        Logger::Log("Swapchain out of date or suboptimal, recreating on frame " + std::to_string(m_currentFrame));
    } else if (result != VK_SUCCESS)
    {
        Logger::LogError("Failed to present swapchain image: " + std::to_string(result));
    }

    NextFrameIndex();
}

void Renderer::SubmitRender() const
{
    m_swapchainImages[m_currentImageIndex]->MakePresent();

    constexpr VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

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
        Logger::LogError("Failed to submit command buffer");
    }
}

void Renderer::EndRecord() const
{
    VkCommandBuffer commandBuffer = m_commandBuffers[m_currentFrame];
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
    {
        Logger::LogError("Failed to end command buffer recording");
    }
}

bool Renderer::CreateSwapchain()
{
    // Query surface capabilities
    // ^ Ensures that moving to a different monitor with a different colorspace
    // ^ or requirements doesn't cause any problems
    m_surfaceFormat = Utils::Renderer::ChooseBestSurfaceFormat(m_device.GetPhysicalDevice(), m_surface);
    m_presentMode = Utils::Renderer::ChooseBestPresentMode(m_device.GetPhysicalDevice(), m_surface);
    m_swapchainImageFormat = m_surfaceFormat.format;
    m_swapchainExtent = Utils::Renderer::ChooseBestExtent(m_device.GetPhysicalDevice(), m_surface, m_window->GetWindowWidth(), m_window->GetWindowHeight());

    const uint32_t minImageCount = Utils::Renderer::GetMinImageCount(m_device.GetPhysicalDevice(), m_surface);

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
    if (vkCreateSwapchainKHR(m_device.GetLogicalDevice(), &swapchainInfo, nullptr, &newSwapchain) != VK_SUCCESS)
    {
        Logger::LogError("Failed to create swapchain");
        return false;
    }

    if (m_swapchain)
    {
        DestroySwapchain();
        DestroySwapchainImages();
    }

    m_swapchain = newSwapchain;

    return true;
}

bool Renderer::CreateSwapchainImages()
{
    uint32_t imageCount = 0;
    vkGetSwapchainImagesKHR(m_device.GetLogicalDevice(), m_swapchain, &imageCount, nullptr);
    std::vector<VkImage> swapchainVkImages(imageCount);
    vkGetSwapchainImagesKHR(m_device.GetLogicalDevice(), m_swapchain, &imageCount, swapchainVkImages.data());

    m_swapchainImages.reserve(imageCount);

    for (uint32_t i = 0; i < imageCount; ++i)
    {
        auto image = new Resources::Image(m_device);

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

        if (vkCreateImageView(m_device.GetLogicalDevice(), &viewInfo, nullptr, &imageView) != VK_SUCCESS)
        {
            Logger::LogError("Failed to create image view");
            return false;
        }

        image->SetAspectMask(VK_IMAGE_ASPECT_COLOR_BIT);

        if (!image->Init(swapchainVkImages[i], imageView))
        {
            Logger::LogError("Failed to initialize swapchain image");
            return false;
        }

        image->MakePresent();
        m_swapchainImages.push_back(image);
    }
    return true;
}

bool Renderer::CreateImages()
{
    for (uint32_t i = 0; i < m_device.GetMaxFramesInFlight(); ++i)
    {
        auto colorImage = new Resources::Image(m_device);
        auto depthImage = new Resources::Image(m_device);
        auto pipelineImage = new Resources::Image(m_device);

        colorImage->SetAspectMask(VK_IMAGE_ASPECT_COLOR_BIT)
            .SetUsage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
            .SetExtent({m_swapchainExtent.width, m_swapchainExtent.height, 1})
            .SetFormat(m_swapchainImageFormat);

        depthImage->SetAspectMask(VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)
            .SetUsage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
            .SetExtent({m_swapchainExtent.width, m_swapchainExtent.height, 1})
            .SetFormat(m_device.GetPreferredDepthFormat());

        pipelineImage->SetAspectMask(VK_IMAGE_ASPECT_COLOR_BIT)
            .SetUsage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
            .SetExtent({m_swapchainExtent.width, m_swapchainExtent.height, 1})
            .SetFormat(m_swapchainImageFormat);

        if (!colorImage->Init(VK_NULL_HANDLE, VK_NULL_HANDLE))
        {
            Logger::LogError("Failed to create color image");
            return false;
        }
        if (!depthImage->Init(VK_NULL_HANDLE, VK_NULL_HANDLE))
        {
            Logger::LogError("Failed to create depth image");
            return false;
        }
        if (!pipelineImage->Init(VK_NULL_HANDLE, VK_NULL_HANDLE))
        {
            Logger::LogError("Failed to create pipeline image");
            return false;
        }

        m_colorImages.push_back(colorImage);
        m_depthImages.push_back(depthImage);
        m_pipelineImages.push_back(pipelineImage);
    }
    return true;
}

bool Renderer::CreateSampler()
{
    auto* sampler = new Resources::Sampler(m_device);
    sampler->SetMagFilter(VK_FILTER_LINEAR)
           .SetMinFilter(VK_FILTER_LINEAR)
           .SetAddressModeU(VK_SAMPLER_ADDRESS_MODE_REPEAT)
           .SetAddressModeV(VK_SAMPLER_ADDRESS_MODE_REPEAT)
           .SetAddressModeW(VK_SAMPLER_ADDRESS_MODE_REPEAT)
           .SetAnisotropyLevel(1.0f)
           .SetAnisotropy(false);
    if (!sampler->Init())
    {
        Logger::LogError("Failed to create sampler");
        delete sampler;
        m_sampler = nullptr;
        return false;
    }
    m_sampler = sampler;
    return true;
}

bool Renderer::CreateCommandBuffers()
{
    m_commandBuffers.resize(m_device.GetMaxFramesInFlight());

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_device.GetCommandPool();
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size());

    if (vkAllocateCommandBuffers(m_device.GetLogicalDevice(), &allocInfo, m_commandBuffers.data()) != VK_SUCCESS)
    {
        Logger::LogError("Failed to allocate command buffers");
        return false;
    }
    return true;
}

bool Renderer::CreateSyncObjects()
{
    m_imageAvailableSemaphores.resize(m_device.GetMaxFramesInFlight());
    m_renderFinishedSemaphores.resize(m_device.GetMaxFramesInFlight());
    m_inFlightFences.resize(m_device.GetMaxFramesInFlight());

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < m_device.GetMaxFramesInFlight(); ++i)
    {
        if (vkCreateSemaphore(m_device.GetLogicalDevice(), &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(m_device.GetLogicalDevice(), &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(m_device.GetLogicalDevice(), &fenceInfo, nullptr, &m_inFlightFences[i]) != VK_SUCCESS)
        {
            Logger::LogError("Failed to create sync objects");
            return false;
        }
    }
    return true;
}

void Renderer::DestroyCommandBuffers()
{
    for (auto& commandBuffer : m_commandBuffers)
    {
        if (commandBuffer != VK_NULL_HANDLE)
        {
            vkFreeCommandBuffers(m_device.GetLogicalDevice(), m_device.GetCommandPool(), 1, &commandBuffer);
            commandBuffer = VK_NULL_HANDLE;
        }
    }
    m_commandBuffers.clear();
}

void Renderer::DestroySyncObjects()
{
    for (size_t i = 0; i < m_device.GetMaxFramesInFlight(); ++i)
    {
        if (m_imageAvailableSemaphores[i] != VK_NULL_HANDLE)
        {
            vkDestroySemaphore(m_device.GetLogicalDevice(), m_imageAvailableSemaphores[i], nullptr);
            m_imageAvailableSemaphores[i] = VK_NULL_HANDLE;
        }
        if (m_renderFinishedSemaphores[i] != VK_NULL_HANDLE)
        {
            vkDestroySemaphore(m_device.GetLogicalDevice(), m_renderFinishedSemaphores[i], nullptr);
            m_renderFinishedSemaphores[i] = VK_NULL_HANDLE;
        }
        if (m_inFlightFences[i] != VK_NULL_HANDLE)
        {
            vkDestroyFence(m_device.GetLogicalDevice(), m_inFlightFences[i], nullptr);
            m_inFlightFences[i] = VK_NULL_HANDLE;
        }
    }
}

void Renderer::DestroySwapchainImages()
{
    for (auto& image : m_swapchainImages)
    {
        if (image)
        {
            image->Cleanup();
            delete image;
            image = nullptr;
        }
    }
    m_swapchainImages.clear();
}

void Renderer::DestroySampler()
{
    if (m_sampler)
    {
        m_sampler->Cleanup();
        delete m_sampler;
        m_sampler = nullptr;
    }
}

void Renderer::DestroyImages()
{
    for (auto& image : m_colorImages)
    {
        if (image)
        {
            image->Cleanup();
            delete image;
            image = nullptr;
        }
    }
    m_colorImages.clear();

    for (auto& image : m_depthImages)
    {
        if (image)
        {
            image->Cleanup();
            delete image;
            image = nullptr;
        }
    }
    m_depthImages.clear();

    for (auto& image : m_pipelineImages)
    {
        if (image)
        {
            image->Cleanup();
            delete image;
            image = nullptr;
        }
    }
    m_pipelineImages.clear();
}

void Renderer::DestroySwapchain()
{
    if (m_swapchain != VK_NULL_HANDLE)
    {
        vkDestroySwapchainKHR(m_device.GetLogicalDevice(), m_swapchain, nullptr);
        m_swapchain = VK_NULL_HANDLE;
    }
}
}
