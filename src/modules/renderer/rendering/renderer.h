//
// Created by lepag on 6/8/2025.
//

#pragma once

#include <volk.h>

#include "../utilities/renderer.h"
#include "viewport.h"
#include "../../platform/window.h"
#include "resources/image.h"

class RenderingDevice;

class Renderer {
public:
    explicit Renderer(RenderingDevice& device) : m_device(device) {}
    ~Renderer();

    bool init(Window* window);
    void cleanup();

    bool recreate();
    void advanceFrame();

    VkCommandBuffer beginFrame();
    void setViewport(Viewport viewport);
    void renderFrame();
    void endFrame();
private:
    RenderingDevice& m_device;
    Window* m_window = nullptr;

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
