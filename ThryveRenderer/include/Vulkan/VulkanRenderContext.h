//
// Created by kprie on 14.03.2024.
//
#pragma once
#include "pch.h"
#include "ThreadPool.h"
#include "VulkanFrameSynchronizer.h"
#include "VulkanRenderPassBuilder.h"
#include "Vertex2D.h"
#include "VulkanCommandBuffer.h"
#include "VulkanCommandPoolManager.h"
#include "VulkanDescriptorManager.h"
#include "VulkanInstance.h"
#include "VulkanDeviceSelector.h"
#include "VulkanIndexBuffer.h"
#include "VulkanPipeline.h"
#include "VulkanSwapChain.h"
#include "VulkanTextureImage.h"
#include "VulkanVertexBuffer.h"
#include "VulkanWindowContext.h"
#include "../../IGraphicsContext.h"
#include "GLFW/glfw3.h"
#include "glm/ext/matrix_transform.hpp"
#include "spdlog/spdlog.h"

constexpr uint32_t WIDTH = 1920;
constexpr uint32_t HEIGHT = 1080;

constexpr int MAX_FRAMES_IN_FLIGHT = 2;

const std::vector<const char *> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif

const std::vector<Vertex2D> vertices = {
    {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f},{1.0f, 0.0f}},
    {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f},{0.0f,0.0f}},
    {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f,1.0f}},
    {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f,1.0f}},
};

const std::vector<uint32_t> indices = {
    0, 1, 2, 2, 3, 0
};

class VulkanRenderContext final : public IGraphicsContext {
public:
    VulkanRenderContext();
    ~VulkanRenderContext();

    void run() override;

private:
    // Window management
    GLFWwindow* window;
    std::unique_ptr<VulkanWindowContext> m_WindowContext;

    // Vulkan core components
    std::unique_ptr<VulkanInstance> m_Instance;
    VkSurfaceKHR surface;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    std::unique_ptr<VulkanDeviceSelector> m_DeviceSelector;
    VkDevice device;

    // Swap chain and rendering setup
    std::unique_ptr<VulkanSwapChain> m_SwapChain;
    VkRenderPass renderPass;
    std::unique_ptr<VulkanRenderPassBuilder> m_RenderPassFactory;
    std::unique_ptr<VulkanPipeline> m_Pipeline;
    VkFramebuffer framebuffer;

    // Command processing
    VkCommandPool commandPool;
    std::unique_ptr<VulkanCommandPoolManager> m_CmdPoolManager;
    VkCommandBuffer commandBuffer;
    std::unique_ptr<VulkanCommandBuffer> m_CmdBuffer;

    // Buffers, vertices, and indices
    std::unique_ptr<VulkanVertexBuffer<Vertex2D>> m_VulkanVertexBuffer;
    std::unique_ptr<VulkanIndexBuffer> m_IndexBuffer;

    // Descriptor sets and buffers
    VkDescriptorSetLayout m_descriptorSetLayout;
    VkDescriptorPool m_descriptorPool;
    std::vector<VkDescriptorSet> m_descriptorSets;
    std::vector<VkBuffer> m_uniformBuffers;
    std::vector<VkDeviceMemory> m_uniformBuffersMemory;
    std::vector<void*> m_uniformBuffersMapped;
    std::unique_ptr<VulkanDescriptorManager> m_descriptorManager;

    // Synchronization
    std::unique_ptr<VulkanFrameSynchronizer> m_FrameSynchronizer;
    uint32_t currentFrame = 0;

    //Texture Creation
    std::unique_ptr<VulkanTextureImage> m_VulkanTextureImage;
    VkImage m_TextureImage;
    VkImageView m_TextureImageView;
    VkSampler m_TextureSampler;

    // Initialization and setup methods
    void initWindow();


    void initVulkan();
    void createSurface();
    void pickSuitableDevices();
    void initInstance();
    void createSwapChain();
    void initRenderPassFactory();
    void createGraphicsPipeline();
    void createFramebuffers();
    void createCommandPool();
    void initCmdBufferManager();
    void createVertexBuffer();
    void createIndexBuffer();
    void createUniformBuffer();
    void createDescriptorSetLayout();
    void createTextureImage();
    void createTextureImageView();
    void createTextureSampler();
    VkDescriptorPool createDescriptorPool() const;
    void CreateDescriptorSets();
    void createCommandBuffer();


    void recordCommandBufferSegment(VkCommandBuffer commandBuffer, uint32_t imageIndex) const;

    // Main loop and frame drawing
    void mainLoop();
    void drawFrame();
    void updateUniformBuffer(uint32_t currentImage) const;

    // Synchronization methods
    void createSyncObjects();

    // Cleanup
    void cleanup();
};
