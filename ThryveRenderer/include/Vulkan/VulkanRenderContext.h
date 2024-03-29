//
// Created by kprie on 14.03.2024.
//
#pragma once

#include "GLFW/glfw3.h"
#include "ThreadPool.h"
#include "Vertex2D.h"
#include "VulkanCommandBuffer.h"
#include "VulkanCommandPoolManager.h"
#include "VulkanDescriptorManager.h"
#include "VulkanDeviceSelector.h"
#include "VulkanFrameSynchronizer.h"
#include "VulkanIndexBuffer.h"
#include "VulkanPipeline.h"
#include "VulkanRenderPassBuilder.h"
#include "VulkanSwapChain.h"
#include "VulkanTextureImage.h"
#include "VulkanVertexBuffer.h"
#include "glm/ext/matrix_transform.hpp"
#include "pch.h"

constexpr uint32_t WIDTH = 1920;
constexpr uint32_t HEIGHT = 1080;

constexpr int MAX_FRAMES_IN_FLIGHT = 2;

const std::vector<Vertex3D> VERTICES_3D = {
    {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f},{1.0f, 0.0f}},
    {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f},{0.0f,0.0f}},
    {{0.5f, 0.5f,0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f,1.0f}},
    {{-0.5f, 0.5f,0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f,1.0f}},

    {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f},{1.0f, 0.0f}},
    {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f},{0.0f,0.0f}},
    {{0.5f, 0.5f,-0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f,1.0f}},
    {{-0.5f, 0.5f,-0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f,1.0f}},
};

const std::vector<Vertex2D> VERTICES_2D = {
    {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f},{1.0f, 0.0f}},
    {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f},{0.0f,0.0f}},
    {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f,1.0f}},
    {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f,1.0f}},
};

const std::vector<uint32_t> INDICES_2D = {
    0, 1, 2, 2, 3, 0
};

const std::vector<uint32_t> INDICES_3D = {
    0, 1, 2, 2, 3, 0,
    4, 5, 6, 6, 7, 4
};

namespace Thryve::Rendering
{
    class VulkanRenderContext final : public Core::ReferenceCounted {
    public:
        VulkanRenderContext();
        ~VulkanRenderContext() override;

        void Run();

    private:
        // Vulkan core components
        VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
        VkDevice m_device;

        VkImage depthImage;
        VkDeviceMemory dethImageMemory;
        VkImageView depthimageView;

        // Depth Functions
        void CreateDepthResources();

        std::vector<Vertex3D> ModelVertices;
        std::vector<uint32_t> ModelIndices;
        VkBuffer vertexBufer;
        VkDeviceMemory vertexBufferMemory;

        void LoadModel(const std::string& path);

        // Swap chain and rendering setup
        std::unique_ptr<VulkanSwapChain> m_swapChain;
        VkRenderPass m_renderPass;
        std::unique_ptr<VulkanRenderPassBuilder> m_renderPassFactory;
        std::unique_ptr<VulkanPipeline> m_pipeline;
        VkFramebuffer m_framebuffer;

        // Command processing
        VkCommandPool m_commandPool;
        std::unique_ptr<VulkanCommandPoolManager> m_cmdPoolManager;
        VkCommandBuffer m_commandBuffer;
        std::unique_ptr<VulkanCommandBuffer> m_cmdBuffer;

        // Buffers, vertices, and indices
        std::unique_ptr<VulkanVertexBuffer<Vertex3D>> m_vulkanVertexBuffer;
        std::unique_ptr<VulkanIndexBuffer> m_indexBuffer;

        // Descriptor sets and buffers
        VkDescriptorSetLayout m_descriptorSetLayout;
        VkDescriptorPool m_descriptorPool;
        std::vector<VkDescriptorSet> m_descriptorSets;
        std::vector<VkBuffer> m_uniformBuffers;
        std::vector<VkDeviceMemory> m_uniformBuffersMemory;
        std::vector<void*> m_uniformBuffersMapped;
        Core::UniqueRef<VulkanDescriptorManager> m_descriptorManager;

        // Synchronization
        std::unique_ptr<VulkanFrameSynchronizer> m_FrameSynchronizer;
        uint32_t currentFrame = 0;

        //Texture Creation
        std::unique_ptr<VulkanTextureImage> m_VulkanTextureImage;
        VkImage m_textureImage;
        VkImageView m_textureImageView;
        VkSampler m_textureSampler;


        void InitVulkan();
        void PickSuitableDevices();
        void CreateSwapChain();
        void InitRenderPassFactory();
        void CreateGraphicsPipeline();
        void CreateFramebuffers();
        void CreateCommandPool();
        void InitCmdBufferManager();
        void CreateVertexBuffer();
        void CreateIndexBuffer();
        void CreateUniformBuffer();
        void CreateDescriptorSetLayout();
        void CreateTextureImage(const std::string &path);
        void CreateTextureImageView();
        void CreateTextureSampler();
        [[nodiscard]] VkDescriptorPool CreateDescriptorPool() const;
        void CreateDescriptorSets();
        void CreateCommandBuffer();

        void RecordCommandBufferSegment(VkCommandBuffer commandBuffer, uint32_t imageIndex) const;
        // Main loop and frame drawing
        void MainLoop();
        void RecreateSwapChain();
        void DrawFrame();
        void UpdateUniformBuffer(uint32_t currentImage) const;
        // Synchronization methods
        void CreateSyncObjects();
        // Cleanup
        void Cleanup();
    };
}
