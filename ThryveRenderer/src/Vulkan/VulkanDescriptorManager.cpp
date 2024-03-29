//
// Created by kprie on 15.03.2024.
//

#include "Vulkan/VulkanDescriptorManager.h"

#include "Vulkan/VulkanContext.h"
#include "Vulkan/VulkanDescriptorSetBuilder.h"
#include "utils/VkDebugUtils.h"

namespace Thryve::Rendering {
    VulkanDescriptorManager::VulkanDescriptorManager(VkDescriptorPool descriptorPool): m_descriptorPool(descriptorPool), m_descriptorSetLayout(nullptr) {
        m_device = Thryve::Rendering::VulkanContext::GetCurrentDevice()->GetLogicalDevice();
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

    // In VulkanDescriptorManager class, add these helper methods:
    VkWriteDescriptorSet VulkanDescriptorManager::createBufferDescriptorWrite(VkDescriptorSet descriptorSet, uint32_t dstBinding, VkDescriptorBufferInfo* bufferInfo) {
        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = descriptorSet;
        descriptorWrite.dstBinding = dstBinding;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = bufferInfo;
        descriptorWrite.pTexelBufferView = nullptr; // Optional
        return descriptorWrite;
    }

    VkWriteDescriptorSet VulkanDescriptorManager::createImageDescriptorWrite(VkDescriptorSet descriptorSet, uint32_t dstBinding, VkDescriptorImageInfo* imageInfo) {
        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = descriptorSet;
        descriptorWrite.dstBinding = dstBinding;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pImageInfo = imageInfo; // Optional
        descriptorWrite.pTexelBufferView = nullptr; // Optional
        return descriptorWrite;
    }

    void VulkanDescriptorManager::UpdateDescriptorSets(const std::vector<VkWriteDescriptorSet> &writeSets) const
    {
        vkUpdateDescriptorSets(m_device, static_cast<uint32_t>(writeSets.size()), writeSets.data(), 0, nullptr);
    }
}
