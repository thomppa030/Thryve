//
// Created by kprie on 14.03.2024.
//

#include "Vulkan/VulkanSwapChain.h"

#include <iostream>

#include "Vulkan/VulkanDeviceSelector.h"
#include "GLFW/glfw3.h"

VulkanSwapChain::VulkanSwapChain(VulkanDeviceSelector* deviceSelector, VkSurfaceKHR surface, GLFWwindow* window) : m_deviceSelector(
        deviceSelector), m_surface(surface), m_renderPass(nullptr), m_swapChain(nullptr), m_swapChainImageFormat(),
    m_swapChainExtent() {
    m_physicalDevice = m_deviceSelector->GetPhysicalDevice();
    m_device = m_deviceSelector->GetLogicalDevice();
    m_window = window;
}

VulkanSwapChain::~VulkanSwapChain() {
    CleanupSwapChain();
}

bool VulkanSwapChain::HandlePresentResult(const VkResult result) {
    switch (result) {
        case VK_SUCCESS:
            // The image was presented successfully. No special action required.
                return true;

        case VK_SUBOPTIMAL_KHR:
            // The swap chain can still be used to successfully present to the surface, but the surface properties
                // are no longer exactly matched. Consider recreating the swap chain if you want an optimal presentation.
                    // However, this is not critical.
                        std::cout << "Swap chain is suboptimal." << std::endl;
        return true;

        case VK_ERROR_OUT_OF_DATE_KHR:
            // The swap chain has become incompatible with the surface and can no longer be used for presentation.
                // This typically happens after a window resize. The swap chain needs to be recreated.
                    std::cout << "Swap chain is out of date (e.g., due to window resize). Recreating swap chain." << std::endl;
        return false; // Indicate the need for swap chain recreation.

        default:
            // An unexpected error occurred during presentation.
                throw std::runtime_error("Failed to present swap chain image!");
    }
}

VulkanSwapChain::VulkanSwapChain(VulkanSwapChain &&other) noexcept : m_deviceSelector(other.m_deviceSelector),
                                                                     m_physicalDevice(other.m_physicalDevice),
                                                                     m_device(other.m_device),
                                                                     m_surface(other.m_surface),
                                                                     m_window(other.m_window),
                                                                     m_swapChain(other.m_swapChain),
                                                                     m_swapChainImageFormat(),
                                                                     m_swapChainExtent()
{
    other.m_deviceSelector = nullptr;
    other.m_device = VK_NULL_HANDLE;
    other.m_surface = VK_NULL_HANDLE;
    other.m_swapChain = VK_NULL_HANDLE;
    other.m_physicalDevice = VK_NULL_HANDLE;
    other.m_device = VK_NULL_HANDLE;
}

VulkanSwapChain & VulkanSwapChain::operator=(VulkanSwapChain && other) noexcept {
    if (this != &other) {
        if (m_swapChain != VK_NULL_HANDLE) {
            vkDestroySwapchainKHR(m_device, m_swapChain, nullptr);
        }
    }
    m_device = other.m_device;
    m_surface = other.m_surface,
    m_window = other.m_window,
    m_deviceSelector = other.m_deviceSelector,
    m_physicalDevice = other.m_physicalDevice,
    m_swapChain = other.m_swapChain,
    m_swapChainImageFormat = other.m_swapChainImageFormat,
    m_swapChainExtent = other.m_swapChainExtent;

    other.m_deviceSelector = nullptr;
    other.m_device = VK_NULL_HANDLE;
    other.m_surface = VK_NULL_HANDLE;
    other.m_swapChain = VK_NULL_HANDLE;
    other.m_physicalDevice = VK_NULL_HANDLE;
    other.m_device = VK_NULL_HANDLE;

    return  *this;
}

void VulkanSwapChain::InitializeSwapChain(uint32_t width, uint32_t height) {
    CreateSwapChain(width, height);
    CreateImageViews();
}

void VulkanSwapChain::CleanupSwapChain() const {
    for (const auto framebuffer: m_Framebuffers) {
        vkDestroyFramebuffer(m_device, framebuffer, nullptr);
    }

    for (const auto imageView: m_swapChainImageViews) {
        vkDestroyImageView(m_device, imageView, nullptr);
    }

    vkDestroySwapchainKHR(m_device, m_swapChain, nullptr);
}

void VulkanSwapChain::CreateFramebuffers() {
    m_Framebuffers.resize(m_swapChainImageViews.size());

    for (size_t i = 0; i < m_swapChainImageViews.size(); i++) {
        const VkImageView attachments[] = {
            m_swapChainImageViews[i]
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_renderPass;/* The render pass that your framebuffers will be used with */;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = m_swapChainExtent.width;
        framebufferInfo.height = m_swapChainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(m_device, &framebufferInfo, nullptr, &m_Framebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}

void VulkanSwapChain::RecreateSwapChain() {
        int _width = 0, _height = 0;
        glfwGetFramebufferSize(m_window, &_width, &_height);
        while (_width == 0 || _height == 0) {
            glfwGetFramebufferSize(m_window, &_width, &_height);
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(m_device);

        CleanupSwapChain();
        InitializeSwapChain(_width,_height);
        CreateFramebuffers();
}

void VulkanSwapChain::CreateSwapChain(uint32_t width, uint32_t height) {
        SwapChainSupportDetails swapChainSupport = m_deviceSelector->querySwapChainSupport(m_physicalDevice);

        const VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
        const VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.present_modes);
        const VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = m_surface;

        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        auto [graphicsFamily, presentFamily] = m_deviceSelector->FindQueueFamilies(m_physicalDevice);
        const uint32_t queueFamilyIndices[] = {graphicsFamily.value(), presentFamily.value()};

        if (graphicsFamily != presentFamily) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        } else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;

        if (vkCreateSwapchainKHR(m_device, &createInfo, nullptr, &m_swapChain) != VK_SUCCESS) {
            throw std::runtime_error("failed to create swap chain!");
        }

        vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, nullptr);
        m_swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, m_swapChainImages.data());

        m_swapChainImageFormat = surfaceFormat.format;
        m_swapChainExtent = extent;
}

void VulkanSwapChain::CreateImageViews() {
        m_swapChainImageViews.resize(m_swapChainImages.size());

        for (size_t i = 0; i < m_swapChainImages.size(); i++) {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = m_swapChainImages[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = m_swapChainImageFormat;
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(m_device, &createInfo, nullptr, &m_swapChainImageViews[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create image views!");
            }
        }
}

VkSurfaceFormatKHR VulkanSwapChain::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) {

        for (const auto& availableFormat : availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }

        return availableFormats[0];

}

VkPresentModeKHR VulkanSwapChain::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes) {

        for (const auto& availablePresentMode : availablePresentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return availablePresentMode;
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR;

}

VkExtent2D VulkanSwapChain::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        } else {
            int width, height;
            glfwGetFramebufferSize(m_window, &width, &height);

            VkExtent2D actualExtent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
            };

            actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            return actualExtent;
        }
}

std::pair<VkResult, std::optional<uint32_t>> VulkanSwapChain::AcquireNextImage(VkSemaphore imageAvailableSemaphore) {
    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(m_device, m_swapChain, UINT64_MAX,
                                            imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        // The swap chain is no longer optimal or has become out of date, recreate it
        RecreateSwapChain();
        // Skip the current frame as the swap chain is being recreated
        return {VK_SUCCESS, std::nullopt};
    } else if (result != VK_SUCCESS) {
        throw std::runtime_error("Failed to acquire next image!");
    }
    return {result, imageIndex};
}

bool VulkanSwapChain::HandleAcquireResult(VkResult result) {
    switch (result) {
        case VK_SUCCESS:
        case VK_SUBOPTIMAL_KHR:
            // Success or suboptimal, but still possible to present images.
            return true;
        case VK_ERROR_OUT_OF_DATE_KHR:
            // Swap chain is out of date (e.g., the window was resized) and needs to be recreated.
            return false;
        default:
            // An unexpected error occurred during image acquisition.
                throw std::runtime_error("Failed to acquire swap chain image!");
    }
}

void VulkanSwapChain::SetRenderPass(VkRenderPass renderPass) {
    m_renderPass = renderPass;
}


VkResult VulkanSwapChain::PresentImage(uint32_t imageIndex, VkSemaphore renderFinishedSemaphore) const {
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &renderFinishedSemaphore;

    const VkSwapchainKHR swapChains[] = {m_swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    return vkQueuePresentKHR(m_deviceSelector->GetPresentQueue(), &presentInfo);
}
