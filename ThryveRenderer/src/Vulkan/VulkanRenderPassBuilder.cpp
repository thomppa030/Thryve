//
// Created by kprie on 14.03.2024.
//

#include "Vulkan/VulkanRenderPassBuilder.h"

VulkanRenderPassBuilder::VulkanRenderPassBuilder(const VkDevice device) : m_device(device){
}

VulkanRenderPassBuilder::~VulkanRenderPassBuilder() {
    CleanupRenderPasses();
}

VulkanRenderPass * VulkanRenderPassBuilder::GetRenderPass(const std::string &key) {
    auto it = m_renderPasses.find(key);
    if (it != m_renderPasses.end()) {
        return it->second.get();
    }
    return nullptr;
}

void VulkanRenderPassBuilder::CreateStandardRenderPasses(const VkFormat swapChainImageFormat) {

        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = swapChainImageFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        CreateCustomRenderPass({colorAttachment}, {subpass}, {dependency}, "default");
}

std::shared_ptr<VulkanRenderPass> VulkanRenderPassBuilder::CreateCustomRenderPass(const std::vector<VkAttachmentDescription> &attachments
    , const std::vector<VkSubpassDescription> &subpasses, const std::vector<VkSubpassDependency> &dependencies
    , const std::string &key) {

    auto renderPass = std::make_shared<VulkanRenderPass>(m_device);
    renderPass->Initialize(attachments, subpasses, dependencies);
    m_renderPasses[key] = std::move(renderPass);

    return renderPass;
}

void VulkanRenderPassBuilder::CleanupRenderPasses() {
    m_renderPasses.clear();
}
