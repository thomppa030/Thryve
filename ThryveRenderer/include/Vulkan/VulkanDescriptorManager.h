//
// Created by kprie on 15.03.2024.
//
#pragma once

#include "VulkanDescriptor.h"

class VulkanDescriptorManager {
    public:
    VulkanDescriptorManager(VkDevice device, VkDescriptorPool descriptorPool);

    // Disallow copying to avoid issues with Vulkan handle ownership
    VulkanDescriptorManager(const VulkanDescriptorManager&) = delete;
    VulkanDescriptorManager& operator=(const VulkanDescriptorManager&) = delete;

    // Allow move operations
    VulkanDescriptorManager(VulkanDescriptorManager&&) noexcept = default;
    VulkanDescriptorManager& operator=(VulkanDescriptorManager&&) noexcept = default;

    ~VulkanDescriptorManager();

    void CreateDescriptorSetLayout(const std::vector<VulkanDescriptor>& descriptors);
    void AllocateDescriptorSets(uint32_t setCount);
    // Function to update the descriptor sets with actual resources
    void UpdateDescriptorSets(const std::vector<VkWriteDescriptorSet>& writeSets) const;

    [[nodiscard]] VkDescriptorSetLayout GetDescriptorSetLayout() const { return m_descriptorSetLayout; }
    [[nodiscard]] const std::vector<VkDescriptorSet>& GetDescriptorSets() const { return m_descriptorSets; }

private:
    VkDevice m_device;
    VkDescriptorPool m_descriptorPool;
    VkDescriptorSetLayout m_descriptorSetLayout;
    std::vector<VkDescriptorSet> m_descriptorSets;
};