//
// Created by kprie on 15.03.2024.
//
#pragma once

#include "Core/Ref.h"
#include "VulkanDescriptor.h"

namespace Thryve::Rendering {
    class VulkanDescriptorManager : public Core::ReferenceCounted {
    public:
        VulkanDescriptorManager(VkDescriptorPool descriptorPool);
        ~VulkanDescriptorManager() = default;

        // Disallow copying to avoid issues with Vulkan handle ownership
        VulkanDescriptorManager(const VulkanDescriptorManager &) = delete;
        VulkanDescriptorManager &operator=(const VulkanDescriptorManager &) = delete;

        // Allow move operations
        VulkanDescriptorManager(VulkanDescriptorManager &&) noexcept = default;
        VulkanDescriptorManager &operator=(VulkanDescriptorManager &&) noexcept = default;

        VkWriteDescriptorSet createBufferDescriptorWrite(VkDescriptorSet descriptorSet, uint32_t dstBinding,
                                                         VkDescriptorBufferInfo *bufferInfo);

        VkWriteDescriptorSet createImageDescriptorWrite(VkDescriptorSet descriptorSet, uint32_t dstBinding,
                                                        VkDescriptorImageInfo *imageInfo);

        void CreateDescriptorSetLayout(const std::vector<VulkanDescriptor> &descriptors);

        void AllocateDescriptorSets(uint32_t setCount);
        // Function to update the descriptor sets with actual resources
        void UpdateDescriptorSets(const std::vector<VkWriteDescriptorSet> &writeSets) const;

        [[nodiscard]] VkDescriptorSetLayout GetDescriptorSetLayout() const { return m_descriptorSetLayout; }
        [[nodiscard]] const std::vector<VkDescriptorSet> &GetDescriptorSets() const { return m_descriptorSets; }

    private:
        VkDevice m_device;
        VkDescriptorPool m_descriptorPool;
        VkDescriptorSetLayout m_descriptorSetLayout;
        std::vector<VkDescriptorSet> m_descriptorSets;
    };
} // namespace myNamespace
