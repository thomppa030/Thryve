#pragma once

#include "Vertex2D.h"
#include "pch.h"

//
// Created by kprie on 14.03.2024.
//

struct PipelineConfigInfo {

    struct VertexInputDescription {
        std::vector<VkVertexInputBindingDescription> bindings;
        std::vector<VkVertexInputAttributeDescription> attributes;
    } vertexInput;

    VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    VkPipelineInputAssemblyStateCreateFlags inputAssemblyFlags = 0;

    VkViewport viewport = {}; // Typically set dynamically
    VkRect2D scissor = {}; // Typically set dynamically
    // Including these as placeholders, actual dynamic setting might be preferred.

    VkBool32 rasterizerDiscardEnable = VK_FALSE;
    VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL;
    VkCullModeFlags cullMode = VK_CULL_MODE_BACK_BIT;
    VkFrontFace frontFace = VK_FRONT_FACE_CLOCKWISE;
    VkPipelineRasterizationStateCreateFlags rasterizationFlags = 0;



    VkBool32 depthTestEnable = VK_TRUE;
    VkBool32 depthWriteEnable = VK_TRUE;
    VkCompareOp depthCompareOp = VK_COMPARE_OP_LESS;
    VkPipelineDepthStencilStateCreateFlags depthStencilFlags = 0;

    VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;

    struct ColorBlendAttachment {
        VkBool32 blendEnable = VK_FALSE;
        VkColorComponentFlags colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                               VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        // Additional blending settings can be added as needed
    } colorBlendAttachment;

    // If dynamic states are used, their flags would be stored here.
    std::vector<VkDynamicState> dynamicStates;

    VkDescriptorSetLayout descriptorSetLayout;

    // TODO Simplifying for example purposes; in practice, you may need more detailed configurations.

    PipelineConfigInfo() = default;

    // TODO Helper functions for common configurations could be added here.

    void SetViewportAndScissor(const uint32_t width, const uint32_t height) {
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(width);
        viewport.height = static_cast<float>(height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        scissor.offset = {0, 0};
        scissor.extent = {width, height};
    }

    void ConfigureVertexInputForSimpleSprite() {
        vertexInput.bindings.resize(1);
        vertexInput.bindings[0] = {0, sizeof(Vertex2D), VK_VERTEX_INPUT_RATE_VERTEX};

        vertexInput.attributes.resize(3);
        vertexInput.attributes[0] = {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex2D, pos)};
        vertexInput.attributes[1] = {1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex2D, color)};
    }

    void ConfigureVertexInputForSimpleMesh() {
        vertexInput.bindings.resize(1);
        vertexInput.bindings[0] = {0, sizeof(Vertex3D), VK_VERTEX_INPUT_RATE_VERTEX};

        vertexInput.attributes.resize(3);
        vertexInput.attributes[0] = {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex3D, pos)};
        vertexInput.attributes[1] = {1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex3D, normal)};
        vertexInput.attributes[2] = {2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex3D, texCoord)};
        vertexInput.attributes[3] = {3, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex3D, tangent)};
        vertexInput.attributes[4] = {4, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex3D, bitangent)};
    }

    void EnableDefaultDepthTesting() {
        depthTestEnable = VK_TRUE;
        depthWriteEnable = VK_TRUE;
        depthCompareOp = VK_COMPARE_OP_LESS; // Closer objects obscure further ones
    }

    void ConfigureMultisampling(const VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT) {
        msaaSamples = sampleCount;
    }

    void EnableDynamicViewportAndLineWidth() {
        dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_LINE_WIDTH, VK_DYNAMIC_STATE_SCISSOR};
    }
};

class VulkanPipeline {

public:
    VulkanPipeline(VkRenderPass renderPass);
    ~VulkanPipeline();

    // Delete copy and move semantics for simplicity and Vulkan handle safety
    VulkanPipeline(const VulkanPipeline&) = delete;
    VulkanPipeline& operator=(const VulkanPipeline&) = delete;
    VulkanPipeline(VulkanPipeline&&) = delete;
    VulkanPipeline& operator=(VulkanPipeline&&) = delete;

    [[nodiscard]] VkPipeline GetPipeline() const {return m_graphicsPipeline;}
    [[nodiscard]] VkPipelineLayout GetPipelineLayout() const { return m_pipelineLayout; }

    VkPipelineDepthStencilStateCreateInfo ConfigureDepthStencil(const PipelineConfigInfo & configInfo);
    
    void CreatePipeline(const std::string& vertexShaderPath, const std::string& fragmentShaderPath, const PipelineConfigInfo& configInfo);
    void Bind(VkCommandBuffer commandBuffer);

    // Additional functionalities like setting dynamic states, if needed

private:
    VkDevice m_device;
    VkPipelineLayout m_pipelineLayout;
    VkRenderPass m_renderPass;
    VkPipeline m_graphicsPipeline;


    VkPipelineVertexInputStateCreateInfo ConfigureVertexInputState(const PipelineConfigInfo& configInfo);
    VkPipelineInputAssemblyStateCreateInfo ConfigureInputAssemblyState(const PipelineConfigInfo& configInfo);
    VkPipelineViewportStateCreateInfo ConfigureViewportState(const PipelineConfigInfo& configInfo);
    VkPipelineRasterizationStateCreateInfo ConfigureRasterizer(const PipelineConfigInfo& configInfo);
    VkPipelineMultisampleStateCreateInfo ConfigureMultisampling(const PipelineConfigInfo& configInfo);
    VkPipelineColorBlendStateCreateInfo ConfigureColorBlending(const PipelineConfigInfo& configInfo, const VkPipelineColorBlendAttachmentState& colorBlendAttachment);

    static std::vector<char> ReadShaderFile(const std::string& filename);
    [[nodiscard]] VkShaderModule createShaderModule(const std::vector<char> &code) const;

    void cleanup() const;
};
