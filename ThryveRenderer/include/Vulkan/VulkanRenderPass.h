//
// Created by kprie on 14.03.2024.
//
#pragma once


class VulkanRenderPass {
public:
    explicit VulkanRenderPass(VkDevice device);
    ~VulkanRenderPass();

    VulkanRenderPass(const VulkanRenderPass&) = delete;
    VulkanRenderPass& operator=(const VulkanRenderPass&) = delete;

    VulkanRenderPass(VulkanRenderPass&& other) noexcept = delete;
    VulkanRenderPass& operator=(VulkanRenderPass&& other) noexcept = delete;

    void Initialize(const std::vector<VkAttachmentDescription>& attachmentDescriptions,
                    const std::vector<VkSubpassDescription>& subpassDescriptions,
                    const std::vector<VkSubpassDependency>& dependencies);
    void Cleanup() const;

    [[nodiscard]] VkRenderPass GetRenderPass() const;

private:
    VkDevice m_device;
    VkRenderPass m_renderPass;

    void CreateRenderPass(const std::vector<VkAttachmentDescription>& attachmentDescriptions,
                          const std::vector<VkSubpassDescription>& subpassDescriptions,
                          const std::vector<VkSubpassDependency>& dependencies);
};