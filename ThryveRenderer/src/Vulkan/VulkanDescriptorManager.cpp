//
// Created by kprie on 15.03.2024.
//

#include "Vulkan/VulkanDescriptorManager.h"

#include "utils/VkDebugUtils.h"
#include "Vulkan/VulkanDescriptorSetBuilder.h"

VulkanDescriptorManager::VulkanDescriptorManager(VkDevice device, VkDescriptorPool descriptorPool): m_device(device),
                                                                                                    m_descriptorPool(descriptorPool), m_descriptorSetLayout(nullptr) {
}

void VulkanDescriptorManager::CreateDescriptorSetLayout(const std::vector<VulkanDescriptor> &descriptors) {
    VulkanDescriptorSetBuilder builder(m_device);
    for (const auto &descriptor: descriptors) {
        builder.AddDescriptor(descriptor);
    }
    m_descriptorSetLayout = builder.BuildLayout();
}

void VulkanDescriptorManager::AllocateDescriptorSets(uint32_t setCount) {
    const std::vector layouts(setCount, m_descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_descriptorPool;
    allocInfo.descriptorSetCount = setCount;
    allocInfo.pSetLayouts = layouts.data();

    m_descriptorSets.resize(setCount);
    VK_CALL(vkAllocateDescriptorSets(m_device, &allocInfo, m_descriptorSets.data()));
}

void VulkanDescriptorManager::UpdateDescriptorSets(const std::vector<VkWriteDescriptorSet> &writeSets) const {
    vkUpdateDescriptorSets(m_device, static_cast<uint32_t>(writeSets.size()), writeSets.data(), 0, nullptr);
}
