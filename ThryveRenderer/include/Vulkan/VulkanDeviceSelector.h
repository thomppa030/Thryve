//
// Created by kprie on 14.03.2024.
//
#pragma once

#include "pch.h"
#include <optional>



struct QueueFamilyIndices {
    std::optional<uint32_t> graphics_family;
    std::optional<uint32_t> present_family;

    [[nodiscard]] bool IsComplete() const {
        return graphics_family.has_value() && present_family.has_value();
    }
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> present_modes;
};

const std::vector<const char*> validation_layers = {
    "VK_LAYER_KHRONOS_validation"
};

class VulkanDeviceSelector {
public:
    // Constructor and Destructor
    VulkanDeviceSelector(VkInstance instance, VkSurfaceKHR surface);
    ~VulkanDeviceSelector();

    // Delete copy constructor and copy assignment operator
    VulkanDeviceSelector(const VulkanDeviceSelector&) = delete;
    VulkanDeviceSelector& operator=(const VulkanDeviceSelector&) = delete;

    // Define move constructor and move assignment operator
    VulkanDeviceSelector(VulkanDeviceSelector&& other) noexcept;
    VulkanDeviceSelector& operator=(VulkanDeviceSelector&& other) noexcept;

    [[nodiscard]] VkDevice GetLogicalDevice();
    [[nodiscard]] VkPhysicalDevice GetPhysicalDevice() const;
    [[nodiscard]] VkQueue GetGraphicsQueue() const;
    [[nodiscard]] VkQueue GetPresentQueue() const;

    void PickSuitableDevice(std::vector<const char*> deviceExtensions, bool enableValidationLayers);

    //TODO Parameter not necessary
    QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device) const;
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) const;

private:

    VkInstance m_instance;
    VkSurfaceKHR m_surface;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_logicalDevice = VK_NULL_HANDLE;

    VkQueue m_graphicsQueue;
    VkQueue m_presentQueue;
    int m_validationLayers;

    bool IsDeviceSuitable(VkPhysicalDevice device, const std::vector<const char*>& deviceExtensions);
    bool CheckDeviceExtensionSupport(VkPhysicalDevice device, const std::vector<const char*>& deviceExtensions);
    void CreateLogicalDevice(VkPhysicalDevice physicalDevice, const std::vector<const char*>& deviceExtensions, bool enableValidationLayers);
};
