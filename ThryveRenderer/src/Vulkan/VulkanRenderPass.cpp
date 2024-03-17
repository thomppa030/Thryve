//
// Created by kprie on 14.03.2024.

#include "Vulkan/VulkanRenderPass.h"

/**
 *
 */
VulkanRenderPass::VulkanRenderPass(const VkDevice device): m_device(device), m_renderPass(nullptr) {
}

VulkanRenderPass::~VulkanRenderPass() {
    Cleanup();
}

void VulkanRenderPass::Initialize(const std::vector<VkAttachmentDescription> &attachmentDescriptions
    , const std::vector<VkSubpassDescription> &subpassDescriptions
    , const std::vector<VkSubpassDependency> &dependencies) {
    CreateRenderPass(attachmentDescriptions, subpassDescriptions, dependencies);
}

void VulkanRenderPass::Cleanup() const {
    if (m_renderPass != VK_NULL_HANDLE) {
        vkDestroyRenderPass(m_device, m_renderPass, nullptr);
    }
}

VkRenderPass VulkanRenderPass::GetRenderPass() const {
    return m_renderPass;
}

void VulkanRenderPass::CreateRenderPass(const std::vector<VkAttachmentDescription> &attachmentDescriptions
    , const std::vector<VkSubpassDescription> &subpassDescriptions
    , const std::vector<VkSubpassDependency> &dependencies) {

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDescriptions.size());
    renderPassInfo.pAttachments = attachmentDescriptions.data();
    renderPassInfo.subpassCount = static_cast<uint32_t>(subpassDescriptions.size());
    renderPassInfo.pSubpasses = subpassDescriptions.data();
    renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
    renderPassInfo.pDependencies = dependencies.data();

    if (vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &m_renderPass) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create render pass!");
    }
}
