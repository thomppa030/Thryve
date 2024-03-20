//
// Created by kprie on 14.03.2024.
//
#pragma once

#include "pch.h"
#include "Core/Ref.h"

class VulkanInstance : public Thryve::Core::ReferenceCounted {
public:
    VulkanInstance();
    ~VulkanInstance();

    // Prevent copy or move semantics for simplicity
    VulkanInstance(const VulkanInstance&) = delete;
    VulkanInstance& operator=(const VulkanInstance&) = delete;
    VulkanInstance(VulkanInstance&&) = delete;
    VulkanInstance& operator=(VulkanInstance&&) = delete;

    void Init(const std::string& applicationName);

    [[nodiscard]] VkInstance GetInstance() const {
        return m_instance;
    }

private:
    VkInstance m_instance = VK_NULL_HANDLE;
    bool m_enableValidationLayers;
    std::vector<const char*> m_validationLayers;
    std::vector<const char*> m_requiredExtensions;

    void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);

    void CreateInstance(const std::string& applicationName);

    [[nodiscard]] bool CheckValidationLayerSupport() const;
    [[nodiscard]] std::vector<const char*> GetRequiredExtensions() const;
    void SetupDebugMessenger();
    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

    VkDebugUtilsMessengerEXT debugMessenger;
};

