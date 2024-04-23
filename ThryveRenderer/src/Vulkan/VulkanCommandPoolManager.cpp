//
// Created by kprie on 14.03.2024.
//
#include "Vulkan/VulkanCommandPoolManager.h"

#include <iostream>

#include "Vulkan/VulkanContext.h"
#include "utils/VkDebugUtils.h"

VulkanCommandPoolManager::VulkanCommandPoolManager() {
    auto _deviceSelector = Thryve::Core::App::Get().GetWindow()->GetRenderContext().As<Thryve::Rendering::VulkanContext>()->GetDevice();
    m_device = _deviceSelector->GetLogicalDevice();
    CreateCommandPool(_deviceSelector->FindQueueFamilies(_deviceSelector->GetPhysicalDevice()).GraphicsFamily.value());
}

VulkanCommandPoolManager::~VulkanCommandPoolManager() {
    DestroyCommandPool();

    for (const auto& commandPool : m_threadCommandPools) {
        vkDestroyCommandPool(m_device, commandPool, nullptr);
    }
}

void VulkanCommandPoolManager::ResetCommandPool(const VkCommandPoolResetFlags flags) const {
    VK_CALL(vkResetCommandPool(m_device, m_commandPool, flags));
}

void VulkanCommandPoolManager::CreateCommandPool(const uint32_t queueFamilyIndex) {
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndex;

    VK_CALL(vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_commandPool));
}

void VulkanCommandPoolManager::DestroyCommandPool() {
    if (m_commandPool != VK_NULL_HANDLE) {
        vkDestroyCommandPool(m_device, m_commandPool, nullptr);
        m_commandPool = VK_NULL_HANDLE;
    }
}
