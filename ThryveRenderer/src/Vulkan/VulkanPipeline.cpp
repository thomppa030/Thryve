//
// Created by kprie on 14.03.2024.
//

#include "Vulkan/VulkanPipeline.h"

#include <fstream>

VulkanPipeline::VulkanPipeline(const VkDevice device
                               , const VkRenderPass renderPass): m_device(
                                                                     device), m_pipelineLayout(
                                                                     nullptr), m_renderPass(
                                                                     renderPass), m_graphicsPipeline(
                                                                     nullptr) {
}

VulkanPipeline::~VulkanPipeline() {
    cleanup();
}

void VulkanPipeline::CreatePipeline(const std::string &vertexShaderPath, const std::string &fragmentShaderPath
                                    , const PipelineConfigInfo &configInfo) {
    const auto vertShaderCode = ReadShaderFile(vertexShaderPath);
    const auto fragShaderCode = ReadShaderFile(fragmentShaderPath);

    const VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
    const VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    const VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT
                                          | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(configInfo.dynamicStates.size());
    dynamicState.pDynamicStates = configInfo.dynamicStates.data();

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &configInfo.descriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 0;

    if (vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    VkPipelineVertexInputStateCreateInfo VertexInputState = ConfigureVertexInputState(configInfo);
    VkPipelineInputAssemblyStateCreateInfo InputAssemblyState = ConfigureInputAssemblyState(configInfo);
    VkPipelineViewportStateCreateInfo ViewPortState = ConfigureViewportState(configInfo);
    VkPipelineRasterizationStateCreateInfo RasterizerState = ConfigureRasterizer(configInfo);
    VkPipelineMultisampleStateCreateInfo MultisampleState = ConfigureMultisampling(configInfo);
    VkPipelineColorBlendStateCreateInfo ColorblendState = ConfigureColorBlending(configInfo, colorBlendAttachment);

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &VertexInputState;
    pipelineInfo.pInputAssemblyState = &InputAssemblyState;
    pipelineInfo.pViewportState = &ViewPortState;
    pipelineInfo.pRasterizationState = &RasterizerState;
    pipelineInfo.pMultisampleState = &MultisampleState;
    pipelineInfo.pColorBlendState = &ColorblendState;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = m_pipelineLayout;
    pipelineInfo.renderPass = m_renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    if (vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_graphicsPipeline) !=
        VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    vkDestroyShaderModule(m_device, fragShaderModule, nullptr);
    vkDestroyShaderModule(m_device, vertShaderModule, nullptr);
}

void VulkanPipeline::Bind(VkCommandBuffer commandBuffer) {
}

std::vector<char> VulkanPipeline::ReadShaderFile(const std::string &filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }

    const auto fileSize = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}

VkPipelineVertexInputStateCreateInfo VulkanPipeline::ConfigureVertexInputState(const PipelineConfigInfo &configInfo) {
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(configInfo.vertexInput.bindings.size());
    vertexInputInfo.pVertexBindingDescriptions = configInfo.vertexInput.bindings.data();
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(configInfo.vertexInput.attributes.size());
    vertexInputInfo.pVertexAttributeDescriptions = configInfo.vertexInput.attributes.data();
    return vertexInputInfo;
}

VkPipelineInputAssemblyStateCreateInfo VulkanPipeline::ConfigureInputAssemblyState(
    const PipelineConfigInfo &configInfo) {
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = configInfo.topology;
    inputAssembly.primitiveRestartEnable = VK_FALSE;
    // Typically disabled, but could be made configurable via configInfo
    return inputAssembly;
}

VkPipelineViewportStateCreateInfo VulkanPipeline::ConfigureViewportState(const PipelineConfigInfo &configInfo) {
    // Assuming dynamic viewport and scissor, so they're not directly set here.
    // This function should be adjusted if static viewport and scissor states are desired.
    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1; // Dynamic state
    viewportState.pViewports = &configInfo.viewport;
    viewportState.scissorCount = 1; // Dynamic state
    viewportState.pScissors = &configInfo.scissor;
    // Note: Actual VkViewport and VkRect2D objects are set dynamically and not here.
    return viewportState;
}

VkPipelineRasterizationStateCreateInfo VulkanPipeline::ConfigureRasterizer(const PipelineConfigInfo &configInfo) {
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE; // Commonly disabled
    rasterizer.rasterizerDiscardEnable = configInfo.rasterizerDiscardEnable;
    rasterizer.polygonMode = configInfo.polygonMode;
    rasterizer.lineWidth = 1.0f; // Default line width
    rasterizer.cullMode = configInfo.cullMode;
    rasterizer.frontFace = configInfo.frontFace;
    rasterizer.depthBiasEnable = VK_FALSE; // Typically disabled, but could be configurable
    return rasterizer;
}

VkPipelineMultisampleStateCreateInfo VulkanPipeline::ConfigureMultisampling(const PipelineConfigInfo &configInfo) {
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE; // Enable if using sample shading
    multisampling.rasterizationSamples = configInfo.msaaSamples;
    return multisampling;
}

VkPipelineColorBlendStateCreateInfo VulkanPipeline::ConfigureColorBlending(const PipelineConfigInfo &configInfo
                                                                           , const VkPipelineColorBlendAttachmentState &
                                                                           colorBlendAttachment) {
    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f; // Optional
    colorBlending.blendConstants[1] = 0.0f; // Optional
    colorBlending.blendConstants[2] = 0.0f; // Optional
    colorBlending.blendConstants[3] = 0.0f; // Optional
    return colorBlending;
}

//TODO dunno, dont like this here in the Pipeline tbh
VkShaderModule VulkanPipeline::createShaderModule(const std::vector<char> &code) const {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(m_device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module!");
    }

    return shaderModule;
}

void VulkanPipeline::cleanup() const {
    if (m_pipelineLayout != VK_NULL_HANDLE) {
        vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
    }

    if (m_graphicsPipeline != VK_NULL_HANDLE) {
        vkDestroyPipeline(m_device, m_graphicsPipeline, nullptr);
    }
}
