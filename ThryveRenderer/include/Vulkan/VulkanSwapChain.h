//
// Created by kprie on 14.03.2024.
//
#pragma once

#include "GLFW/glfw3.h"
#include "VulkanDeviceSelector.h"
#include "pch.h"


class VulkanRenderPassBuilder;
class VulkanCommandBuffer;
class VulkanCommandPoolManager;
namespace Thryve::Rendering {
    class VulkanContext;
}
class VulkanSwapChain final {
public:
    VulkanSwapChain(Thryve::Core::SharedRef<Thryve::Rendering::VulkanContext> context);
    ~VulkanSwapChain();

    bool HandlePresentResult(VkResult result);

    VulkanSwapChain(const VulkanSwapChain&) = delete; // Disable copy operations
    VulkanSwapChain& operator=(const VulkanSwapChain&) = delete;

    VulkanSwapChain(VulkanSwapChain&&) noexcept; // Enable move semantics
    VulkanSwapChain& operator=(VulkanSwapChain&&) noexcept;

    void InitializeSwapChain();
    void CleanupSwapChain(); // For explicit cleanup, can be called before the destructor

    void CreateFramebuffers(VkDevice device);
    void RecreateSwapChain();

    [[nodiscard]] VkSwapchainKHR GetSwapchain() const { return m_swapChain; }
    [[nodiscard]] std::vector<VkImageView> GetSwapchainImageViews() const { return m_swapChainImageViews; }
    [[nodiscard]] VkExtent2D GetSwapchainExtent() const { return m_swapChainExtent; }
    [[nodiscard]] VkFormat GetSwapchainImageFormat() const { return m_swapChainImageFormat; };
    [[nodiscard]] std::vector<VkImage> GetSwapchainImages() const { return m_swapChainImages; }
    [[nodiscard]] std::vector<VkFramebuffer> GetFrameBuffers() const { return m_Framebuffers; }

    std::pair<VkResult, std::optional<uint32_t>> AcquireNextImage(VkSemaphore imageAvailableSemaphore);

    bool HandleAcquireResult(VkResult result);

    void SetRenderPass(VkRenderPass renderPass);

    VkResult PresentImage(uint32_t imageIndex, VkSemaphore renderFinishedSemaphore) const;

    void CreateSwapChain();
    // Depth Functions
    void CreateDepthResources();
    void CreateImageViews(); // Helper method to create image views for the swap chain images

    VkRenderPass GetRenderPass() const { return m_renderPass; }
    VkCommandPool GetCommandPool() const { return m_commandPool; }
    VkCommandBuffer GetCommandBuffer() const { return m_commandBuffer; }

private:
    Thryve::Core::SharedRef<VulkanDeviceSelector> m_deviceSelector;
    std::unique_ptr<VulkanCommandPoolManager> m_commandPoolManager;
    std::unique_ptr<VulkanCommandBuffer> m_vulkanCommandBuffer;
    VkSurfaceKHR m_surface;
    GLFWwindow* m_window;
    VkCommandPool m_commandPool;
    VkCommandBuffer m_commandBuffer;
    VkRenderPass m_renderPass;

    VkSwapchainKHR m_swapChain;
    std::vector<VkImage> m_swapChainImages;

    VkFormat m_swapChainImageFormat;
    VkExtent2D m_swapChainExtent;

    std::vector<VkImageView> m_swapChainImageViews;
    std::vector<VkFramebuffer> m_Framebuffers;
    std::unique_ptr<VulkanRenderPassBuilder> m_renderPassBuilder;

    // Additional helper methods for swap chain creation and management

    // Utility methods for choosing swap chain surface format, present mode, and extent
    VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

    VkImage m_DepthImage;
    VkDeviceMemory m_DepthImageMemory;
    VkImageView m_DepthImageView;
};
