//
// Created by kprie on 14.03.2024.
//
#pragma once

#include "pch.h"

class VulkanInstance {
public:
    VulkanInstance();
    ~VulkanInstance();

    // Prevent copy or move semantics for simplicity
    VulkanInstance(const VulkanInstance&) = delete;
    VulkanInstance& operator=(const VulkanInstance&) = delete;
    VulkanInstance(VulkanInstance&&) = delete;
    VulkanInstance& operator=(VulkanInstance&&) = delete;

    void init(const std::string& applicationName);

    [[nodiscard]] VkInstance getInstance() const {
        return m_instance;
    }

private:
    VkInstance m_instance = VK_NULL_HANDLE;
    bool m_enableValidationLayers;
    std::vector<const char*> m_validationLayers;
    std::vector<const char*> m_requiredExtensions;

    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);

    void createInstance(const std::string& applicationName);

    [[nodiscard]] bool checkValidationLayerSupport() const;
    [[nodiscard]] std::vector<const char*> getRequiredExtensions() const;
    void setupDebugMessenger();
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

    VkDebugUtilsMessengerEXT debugMessenger;
};

