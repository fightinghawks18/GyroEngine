//
// Created by lepag on 6/8/2025.
//

#pragma once

#include <volk.h>

#include "../utilities/renderer.h"
#include "viewport.h"
#include "../../platform/window.h"
#include "../resources/texture/image.h"
#include "../resources/texture/sampler.h"

namespace GyroEngine::Device
{
    class RenderingDevice;
}
using namespace GyroEngine;

namespace GyroEngine::Rendering
{
    struct FrameContext
{
    VkCommandBuffer cmd;

    uint32_t imageIndex;
    uint32_t frameIndex;

    Resources::Image* swapchainImage;
    VkExtent2D swapchainExtent;

    Resources::Sampler* sampler;

    Resources::Image* colorImage;
    Resources::Image* depthImage;
    std::vector<Resources::Image*> pipelineImages;

    Viewport viewport;
};

class Renderer {
public:
    explicit Renderer(Device::RenderingDevice& device) : m_device(device)
    {
    }

    ~Renderer();

    bool Init(Window* window);
    void Cleanup();

    bool Resize();
    void NextFrameIndex();

    bool RecordFrame();
    void BindViewport(const Viewport& viewport);
    void StartRender(const VkRenderingInfoKHR &renderingInfo);
    void EndRender() const;
    void SubmitFrame();

    [[nodiscard]] VkFormat GetSwapchainColorFormat() const
    {
        return m_swapchainImageFormat;
    }

    [[nodiscard]] FrameContext& GetFrameContext()
    {
        m_frameContext.cmd = m_commandBuffers[m_currentFrame];
        m_frameContext.imageIndex = m_currentImageIndex;
        m_frameContext.frameIndex = m_currentFrame;
        m_frameContext.swapchainImage = m_swapchainImages[m_currentImageIndex];
        m_frameContext.viewport = m_viewport;
        m_frameContext.swapchainExtent = m_swapchainExtent;
        m_frameContext.colorImage = m_colorImages[m_currentFrame];
        m_frameContext.depthImage = m_depthImages[m_currentFrame];
        m_frameContext.pipelineImages = m_pipelineImages;
        m_frameContext.sampler = m_sampler;
        return m_frameContext;
    }

    [[nodiscard]] std::vector<Resources::Image*>& GetPipelineImages()
    {
        return m_pipelineImages;
    }
private:
    Device::RenderingDevice& m_device;
    Window* m_window = nullptr;

    VkRenderingInfoKHR m_renderingInfo = {};

    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    VkQueue m_presentQueue = VK_NULL_HANDLE;
    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
    VkFormat m_swapchainImageFormat = VK_FORMAT_UNDEFINED;
    VkSurfaceFormatKHR m_surfaceFormat = {};
    VkExtent2D m_swapchainExtent = {};
    VkPresentModeKHR m_presentMode = VK_PRESENT_MODE_FIFO_KHR;
    std::vector<Resources::Image*> m_swapchainImages = {};
    std::vector<Resources::Image*> m_colorImages = {};
    std::vector<Resources::Image*> m_depthImages = {};
    std::vector<Resources::Image*> m_pipelineImages = {};

    Resources::Sampler* m_sampler = nullptr;

    std::vector<VkSemaphore> m_imageAvailableSemaphores = {};
    std::vector<VkSemaphore> m_renderFinishedSemaphores = {};
    std::vector<VkFence> m_inFlightFences = {};
    std::vector<VkCommandBuffer> m_commandBuffers = {};

    Viewport m_viewport = {0, 0, 1.0f, 1.0f, 1.0f};
    uint32_t m_currentFrame = 0;
    uint32_t m_currentImageIndex = 0;
    bool m_needsRecreation = false;

    FrameContext m_frameContext = {};

    bool StartRecord();
    void PresentRender();
    void SubmitRender() const;
    void EndRecord() const;

    bool CreateSwapchain();
    bool CreateSwapchainImages();
    bool CreateImages();
    bool CreateSampler();
    bool CreateCommandBuffers();
    bool CreateSyncObjects();

    void DestroyCommandBuffers();
    void DestroySyncObjects();
    void DestroySwapchainImages();
    void DestroySampler();
    void DestroyImages();
    void DestroySwapchain();
};
}
