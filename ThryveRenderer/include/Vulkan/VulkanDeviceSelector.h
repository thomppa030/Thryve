//
// Created by kprie on 14.03.2024.
//
#pragma once

#include <optional>
#include "pch.h"

#include "Core/Ref.h"


struct QueueFamilyIndices {
    std::optional<uint32_t> GraphicsFamily;
    std::optional<uint32_t> PresentFamily;

    [[nodiscard]] bool IsComplete() const {
        return GraphicsFamily.has_value() && PresentFamily.has_value();
    }
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR Capabilities;
    std::vector<VkSurfaceFormatKHR> Formats;
    std::vector<VkPresentModeKHR> PresentModes;
};

const std::vector<const char*> VALIDATION_LAYERS = {
    "VK_LAYER_KHRONOS_validation"
};

class VulkanDeviceSelector final : public Thryve::Core::ReferenceCounted {
public:
    // Constructor and Destructor
    VulkanDeviceSelector(VkInstance instance, VkSurfaceKHR surface);
    ~VulkanDeviceSelector() override;

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
    SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device) const;

private:

    VkInstance m_instance;
    VkSurfaceKHR m_surface;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_logicalDevice = VK_NULL_HANDLE;

    VkQueue m_graphicsQueue;
    VkQueue m_presentQueue;
    int m_validationLayers{};

    bool IsDeviceSuitable(VkPhysicalDevice device, const std::vector<const char*>& deviceExtensions);
    bool CheckDeviceExtensionSupport(VkPhysicalDevice device, const std::vector<const char*>& deviceExtensions);
    void CreateLogicalDevice(VkPhysicalDevice physicalDevice, const std::vector<const char*>& deviceExtensions, bool enableValidationLayers);
};
