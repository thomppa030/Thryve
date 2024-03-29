//
// Created by kprie on 14.03.2024.
//
#pragma once

#include "GLFW/glfw3.h"
#include "VulkanDeviceSelector.h"
#include "pch.h"

class VulkanSwapChain {
public:
    VulkanSwapChain();
    ~VulkanSwapChain();

    bool HandlePresentResult(VkResult result);

    void SetDepthImageView(VkImageView depthimageView) { m_DepthImageView = depthimageView; };

    VulkanSwapChain(const VulkanSwapChain&) = delete; // Disable copy operations
    VulkanSwapChain& operator=(const VulkanSwapChain&) = delete;

    VulkanSwapChain(VulkanSwapChain&&) noexcept; // Enable move semantics
    VulkanSwapChain& operator=(VulkanSwapChain&&) noexcept;

    void InitializeSwapChain();
    void CleanupSwapChain() const; // For explicit cleanup, can be called before the destructor

    void CreateFramebuffers();
    void RecreateSwapChain();

    [[nodiscard]] VkSwapchainKHR GetSwapchain() const {return m_swapChain;}
    [[nodiscard]] std::vector<VkImageView> GetSwapchainImageViews() const {return m_swapChainImageViews;}
    [[nodiscard]] VkExtent2D GetSwapchainExtent() const {return m_swapChainExtent;}
    [[nodiscard]] VkFormat GetSwapchainImageFormat()const { return m_swapChainImageFormat; };
    [[nodiscard]] std::vector<VkImage> GetSwapchainImages()const {return m_swapChainImages;}
    [[nodiscard]] std::vector<VkFramebuffer> GetFrameBuffers()const {return m_Framebuffers;}

    std::pair<VkResult, std::optional<uint32_t>> AcquireNextImage(VkSemaphore imageAvailableSemaphore);

    bool HandleAcquireResult(VkResult result);

    void SetRenderPass(VkRenderPass renderPass);

    VkResult PresentImage(uint32_t imageIndex, VkSemaphore renderFinishedSemaphore) const;
    
    void CreateSwapChain();
    void CreateImageViews(); // Helper method to create image views for the swap chain images

private:
    Thryve::Core::SharedRef<VulkanDeviceSelector> m_deviceSelector;
    VkPhysicalDevice m_physicalDevice;
    VkDevice m_device;
    VkSurfaceKHR m_surface;
    GLFWwindow* m_window;
    VkRenderPass m_renderPass;

    VkSwapchainKHR m_swapChain;
    std::vector<VkImage> m_swapChainImages;
    VkImageView m_DepthImageView;
    VkFormat m_swapChainImageFormat;
    VkExtent2D m_swapChainExtent;
    std::vector<VkImageView> m_swapChainImageViews;
    std::vector<VkFramebuffer> m_Framebuffers;

    // Additional helper methods for swap chain creation and management

    // Utility methods for choosing swap chain surface format, present mode, and extent
    VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

};