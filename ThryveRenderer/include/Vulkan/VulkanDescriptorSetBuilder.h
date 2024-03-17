//
// Created by kprie on 15.03.2024.
//
#pragma once

#include "VulkanDescriptor.h"


class VulkanDescriptorSetBuilder {
public:
    explicit VulkanDescriptorSetBuilder(const VkDevice device) : m_device(device){}
    ~VulkanDescriptorSetBuilder() = default;

    VulkanDescriptorSetBuilder& addDescriptor(const VulkanDescriptor& descriptor) {
        m_descriptors.push_back(descriptor);
        return *this;
    }

    [[nodiscard]] VkDescriptorSetLayout buildLayout() const {
        std::vector<VkDescriptorSetLayoutBinding> layoutBindings;

        for (const auto& desc : m_descriptors) {
            VkDescriptorSetLayoutBinding layoutBinding{};
            layoutBinding.binding = desc.GetBinding();
            layoutBinding.descriptorType = desc.GetType();
            layoutBinding.descriptorCount = 1; // Adjust based on use case
            layoutBinding.stageFlags = desc.GetStageFlags();
            layoutBindings.push_back(layoutBinding);
        }

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
        layoutInfo.pBindings = layoutBindings.data();

        VkDescriptorSetLayout descriptorSetLayout;
        if (vkCreateDescriptorSetLayout(m_device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create descriptor set layout!");
        }

        return descriptorSetLayout;
    }

private:
    VkDevice m_device;
    std::vector<VulkanDescriptor> m_descriptors;
};

