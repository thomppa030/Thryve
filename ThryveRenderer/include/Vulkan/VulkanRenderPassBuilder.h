#pragma once
#include "pch.h"

//
// Created by kprie on 14.03.2024.
//

#include <unordered_map>
#include "VulkanRenderPass.h"


class VulkanRenderPassBuilder {
public:
    explicit VulkanRenderPassBuilder(VkDevice device);
    ~VulkanRenderPassBuilder();

    // Disallow copying and moving for simplicity
    VulkanRenderPassBuilder(const VulkanRenderPassBuilder&) = delete;
    VulkanRenderPassBuilder& operator=(const VulkanRenderPassBuilder&) = delete;
    VulkanRenderPassBuilder(VulkanRenderPassBuilder&&) = delete;
    VulkanRenderPassBuilder& operator=(VulkanRenderPassBuilder&&) = delete;

    VulkanRenderPass* GetRenderPass(const std::string& key);

    void CreateStandardRenderPasses(VkFormat swapChainImageFormat);

    std::shared_ptr<VulkanRenderPass> CreateCustomRenderPass(
        const std::vector<VkAttachmentDescription>& attachments,
        const std::vector<VkSubpassDescription>& subpasses,
        const std::vector<VkSubpassDependency>& dependencies,
        const std::string& key);

private:
    VkDevice m_device;
    std::unordered_map<std::string, std::shared_ptr<VulkanRenderPass>> m_renderPasses;

    void CleanupRenderPasses();
};