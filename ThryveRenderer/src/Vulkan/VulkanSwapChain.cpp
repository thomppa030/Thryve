//
// Created by kprie on 14.03.2024.
//

#include "Vulkan/VulkanSwapChain.h"

#include <iostream>

#include "GLFW/glfw3.h"
#include "Vulkan/VulkanContext.h"
#include "Vulkan/VulkanDeviceSelector.h"
#include "utils/ImageUtils.h"

VulkanSwapChain::VulkanSwapChain() :  m_renderPass(nullptr), m_swapChain(nullptr), m_swapChainImageFormat(),
    m_swapChainExtent() {

    m_deviceSelector = Thryve::Rendering::VulkanContext::Get()->GetDevice();
    m_surface = Thryve::Rendering::VulkanContext::GetSurface();
    m_window = Thryve::Rendering::VulkanContext::GetWindowStatic();

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
                                                                     m_surface(other.m_surface),
                                                                     m_window(other.m_window),
                                                                     m_swapChain(other.m_swapChain),
                                                                     m_swapChainImageFormat(),
                                                                     m_swapChainExtent()
{
    other.m_deviceSelector = nullptr;
    other.m_surface = VK_NULL_HANDLE;
    other.m_swapChain = VK_NULL_HANDLE;
}

VulkanSwapChain & VulkanSwapChain::operator=(VulkanSwapChain && other) noexcept {
    if (this != &other) {
        if (m_swapChain != VK_NULL_HANDLE) {
            const VkDevice _device = Thryve::Rendering::VulkanContext::GetCurrentDevice()->GetLogicalDevice();
            vkDestroySwapchainKHR(_device, m_swapChain, nullptr);
        }
    }
    m_surface = other.m_surface,
    m_window = other.m_window,
    m_deviceSelector = other.m_deviceSelector,
    m_swapChain = other.m_swapChain,
    m_swapChainImageFormat = other.m_swapChainImageFormat,
    m_swapChainExtent = other.m_swapChainExtent;

    other.m_deviceSelector = nullptr;
    other.m_surface = VK_NULL_HANDLE;
    other.m_swapChain = VK_NULL_HANDLE;

    return  *this;
}

void VulkanSwapChain::InitializeSwapChain() {
    CreateSwapChain();
}

void VulkanSwapChain::CleanupSwapChain() {
    VkDevice _device = Thryve::Rendering::VulkanContext::GetCurrentDevice()->GetLogicalDevice();

    m_renderPassBuilder.reset();
    m_vulkanCommandBuffer.reset();
    m_commandPoolManager.reset();
    vkDestroyImageView(_device,m_DepthImageView, nullptr);
    vkDestroyImage(_device, m_DepthImage, nullptr);
    vkFreeMemory(_device, m_DepthImageMemory, nullptr);

    for (auto framebuffer: m_Framebuffers) {
        vkDestroyFramebuffer(_device, framebuffer, nullptr);
    }

    for (auto imageView: m_swapChainImageViews) {
        vkDestroyImageView(_device, imageView, nullptr);
    }

    vkDestroySwapchainKHR(_device, m_swapChain, nullptr);
}

void VulkanSwapChain::CreateFramebuffers(VkDevice device) {
    m_Framebuffers.resize(m_swapChainImageViews.size());

    for (size_t i = 0; i < m_swapChainImageViews.size(); i++) {
        std::array<VkImageView, 2> _attachments = {
            m_swapChainImageViews[i],
            m_DepthImageView
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_renderPass;/* The render pass that your framebuffers will be used with */;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(_attachments.size());
        framebufferInfo.pAttachments = _attachments.data();
        framebufferInfo.width = m_swapChainExtent.width;
        framebufferInfo.height = m_swapChainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &m_Framebuffers[i]) != VK_SUCCESS) {
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

        VkDevice _device = Thryve::Rendering::VulkanContext::GetCurrentDevice()->GetLogicalDevice();

        vkDeviceWaitIdle(_device);

        CleanupSwapChain();
        CreateSwapChain();
}

void VulkanSwapChain::CreateSwapChain() {

        VkDevice _device = m_deviceSelector->GetLogicalDevice();
        VkPhysicalDevice _physicalDevice = m_deviceSelector->GetPhysicalDevice();

        SwapChainSupportDetails swapChainSupport = m_deviceSelector->QuerySwapChainSupport(_physicalDevice);

        //TODO Refactor from class to utilityclass, maybe? Could also be too much abstraction
        const VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.Formats);
        const VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.PresentModes);
        const VkExtent2D extent = ChooseSwapExtent(swapChainSupport.Capabilities);

        //TODO Update Dimensions here?

        uint32_t imageCount = swapChainSupport.Capabilities.minImageCount + 3;

        if (swapChainSupport.Capabilities.maxImageCount > 0 && imageCount > swapChainSupport.Capabilities.maxImageCount)
        {
            imageCount = swapChainSupport.Capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR _createInfo{};
        _createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        _createInfo.surface = m_surface;

        _createInfo.minImageCount = imageCount;
        _createInfo.imageFormat = surfaceFormat.format;
        _createInfo.imageColorSpace = surfaceFormat.colorSpace;
        _createInfo.imageExtent = extent;
        _createInfo.imageArrayLayers = 1;
        _createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        auto [graphicsFamily, presentFamily] = m_deviceSelector->FindQueueFamilies(_physicalDevice);
        const uint32_t queueFamilyIndices[] = {graphicsFamily.value(), presentFamily.value()};

        if (graphicsFamily != presentFamily) {
            _createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            _createInfo.queueFamilyIndexCount = 2;
            _createInfo.pQueueFamilyIndices = queueFamilyIndices;
        } else {
            _createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        _createInfo.preTransform = swapChainSupport.Capabilities.currentTransform;
        _createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        _createInfo.presentMode = presentMode;
        _createInfo.clipped = VK_TRUE;
        _createInfo.oldSwapchain = VK_NULL_HANDLE;

        {
            VK_CALL(vkCreateSwapchainKHR(_device, &_createInfo, nullptr, &m_swapChain));
        }

        {
            vkGetSwapchainImagesKHR(_device, m_swapChain, &imageCount, nullptr);
            m_swapChainImages.resize(imageCount);
            vkGetSwapchainImagesKHR(_device, m_swapChain, &imageCount, m_swapChainImages.data());
        }

        m_swapChainImageFormat = surfaceFormat.format;
        m_swapChainExtent = extent;

    CreateImageViews();

    // Create CommandPool
    m_commandPoolManager = std::make_unique<VulkanCommandPoolManager>();
    m_commandPool = m_commandPoolManager->GetCommandPool();

    // Create Command Buffer
    m_vulkanCommandBuffer = std::make_unique<VulkanCommandBuffer>(m_commandPool);
    m_commandBuffer = m_vulkanCommandBuffer->Allocate();
    // Synchronization Objects
    // Create Render Pass
    m_renderPassBuilder = std::make_unique<VulkanRenderPassBuilder>();
    m_renderPassBuilder->CreateStandardRenderPasses(m_swapChainImageFormat);
    m_renderPass = m_renderPassBuilder->GetRenderPass("default")->GetRenderPass();
    // Framebuffer for (every) Swapchain Image
    CreateDepthResources();
    CreateFramebuffers(_device);
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

            const VkDevice _device = Thryve::Rendering::VulkanContext::GetCurrentDevice()->GetLogicalDevice();
            VK_CALL(vkCreateImageView(_device, &createInfo, nullptr, &m_swapChainImageViews[i]));
        }
}

VkSurfaceFormatKHR VulkanSwapChain::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) {
        PROFILE_FUNCTION()
        for (const auto& availableFormat : availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }

        return availableFormats[0];
}

VkPresentModeKHR VulkanSwapChain::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes) {
        PROFILE_FUNCTION()
        for (const auto& availablePresentMode : availablePresentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return availablePresentMode;
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR;

}

VkExtent2D VulkanSwapChain::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) {
        PROFILE_FUNCTION()
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
    VkResult result = vkAcquireNextImageKHR(Thryve::Rendering::VulkanContext::GetCurrentDevice()->GetLogicalDevice(), m_swapChain, UINT64_MAX,
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

void VulkanSwapChain::CreateDepthResources() {
    PROFILE_FUNCTION()
    VkDevice _device = m_deviceSelector->GetLogicalDevice();

    VkFormat _depthFormat = ImageUtils::FindDepthFormat();

    ImageUtils::CreateImage(GetSwapchainExtent().width, GetSwapchainExtent().height,
                            _depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_DepthImage, m_DepthImageMemory);
    m_DepthImageView = ImageUtils::CreateImageView(m_DepthImage, _depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
//    ImageUtils::transitionImageLayout(
//        _device, m_commandPool, m_deviceSelector->GetGraphicsQueue(), m_DepthImage, _depthFormat,
//        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

