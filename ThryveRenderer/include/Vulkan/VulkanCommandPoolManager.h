//
// Created by kprie on 14.03.2024.
//
#pragma once


class VulkanCommandPoolManager {
public:
    VulkanCommandPoolManager(VkDevice device, uint32_t queueFamilyIndex);

    ~VulkanCommandPoolManager();

    // Delete copy constructor and copy assignment operator to prevent copying
    VulkanCommandPoolManager(const VulkanCommandPoolManager &) = delete;

    VulkanCommandPoolManager &operator=(const VulkanCommandPoolManager &) = delete;

    // Public interface for command pool operations
    [[nodiscard]] VkCommandPool GetCommandPool() const { return m_commandPool; }

    void ResetCommandPool(VkCommandPoolResetFlags flags = 0) const;

private:
    VkDevice m_device{};
    VkCommandPool m_commandPool{};
    std::vector<VkCommandPool> m_threadCommandPools{};

    void CreateCommandPool(uint32_t queueFamilyIndex);

    void DestroyCommandPool();
};
