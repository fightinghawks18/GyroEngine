//
// Created by lepag on 6/8/2025.
//

#pragma once

#include <volk.h>

#include "render_pipeline.h"
#include "../utilities/renderer.h"
#include "viewport.h"
#include "../../platform/window.h"
#include "resources/image.h"

class RenderingDevice;

struct FrameContext
{
    VkCommandBuffer cmd;

    uint32_t imageIndex;
    uint32_t frameIndex;

    Image* swapchainImage;
    VkExtent2D swapchainExtent;

    Viewport viewport;
};

class Renderer {
public:
    explicit Renderer(RenderingDevice& device) : m_device(device), m_renderPipeline(*this)
    {
    }

    ~Renderer();

    bool init(Window* window);
    void cleanup();

    bool recreate();
    void advanceFrame();

    VkCommandBuffer beginFrame();
    void setViewport(Viewport viewport);
    void renderFrame();
    void endFrame();

    [[nodiscard]] RenderPipeline& getPipeline()
    {
        return m_renderPipeline;
    }

    [[nodiscard]] VkFormat getSwapchainColorFormat() const
    {
        return m_swapchainImageFormat;
    }

    [[nodiscard]] FrameContext& getFrameContext()
    {
        m_frameContext.cmd = m_commandBuffers[m_currentFrame];
        m_frameContext.imageIndex = m_currentImageIndex;
        m_frameContext.frameIndex = m_currentFrame;
        m_frameContext.swapchainImage = m_swapchainImages[m_currentImageIndex];
        m_frameContext.viewport = m_viewport;
        m_frameContext.swapchainExtent = m_swapchainExtent;
        return m_frameContext;
    }
private:
    RenderingDevice& m_device;
    Window* m_window = nullptr;

    RenderPipeline m_renderPipeline;

    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    VkQueue m_presentQueue = VK_NULL_HANDLE;
    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
    VkFormat m_swapchainImageFormat = VK_FORMAT_UNDEFINED;
    VkSurfaceFormatKHR m_surfaceFormat = {};
    VkExtent2D m_swapchainExtent = {};
    VkPresentModeKHR m_presentMode = VK_PRESENT_MODE_FIFO_KHR;
    std::vector<Image*> m_swapchainImages = {};

    std::vector<VkSemaphore> m_imageAvailableSemaphores = {};
    std::vector<VkSemaphore> m_renderFinishedSemaphores = {};
    std::vector<VkFence> m_inFlightFences = {};
    std::vector<VkCommandBuffer> m_commandBuffers = {};

    Viewport m_viewport = {0, 0, 1.0f, 1.0f, 1.0f};
    uint32_t m_currentFrame = 0;
    uint32_t m_currentImageIndex = 0;
    bool m_needsRecreation = false;

    FrameContext m_frameContext = {};

    VkCommandBuffer startRecord();
    void presentRender();
    void submitRender();
    void endRecord();

    bool createSwapchain();
    bool createSwapchainImages();
    bool createCommandBuffers();
    bool createSyncObjects();

    void destroyCommandBuffers();
    void destroySyncObjects();
    void destroySwapchainImages();
    void destroySwapchain();
};
