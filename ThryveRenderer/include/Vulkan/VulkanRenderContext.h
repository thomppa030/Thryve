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
#include "VulkanInstance.h"
#include "VulkanDeviceSelector.h"
#include "VulkanIndexBuffer.h"
#include "VulkanPipeline.h"
#include "VulkanSwapChain.h"
#include "VulkanVertexBuffer.h"
#include "VulkanWindowContext.h"
#include "../../IGraphicsContext.h"
#include "GLFW/glfw3.h"
#include "glm/ext/matrix_transform.hpp"
#include "spdlog/spdlog.h"

constexpr uint32_t WIDTH = 1920;
constexpr uint32_t HEIGHT = 1080;

constexpr int MAX_FRAMES_IN_FLIGHT = 2;

const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif

const std::vector<Vertex2D> vertices = {
    {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
    {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}},
};

const std::vector<uint32_t> indices = {
    0, 1, 2, 2, 3, 0
};

class VulkanRenderContext final : public IGraphicsContext {
public:
    VulkanRenderContext();
    ~VulkanRenderContext() override = default;

    void run() override;

private:
    std::unique_ptr<VulkanWindowContext> m_WindowContext;
    std::unique_ptr<VulkanInstance> m_Instance;
    std::unique_ptr<VulkanDeviceSelector> m_DeviceSelector;
    std::unique_ptr<VulkanSwapChain> m_SwapChain;
    std::unique_ptr<VulkanRenderPassBuilder> m_RenderPassFactory;
    std::unique_ptr<VulkanPipeline> m_Pipeline;
    std::unique_ptr<VulkanCommandPoolManager> m_CmdPoolManager;
    std::unique_ptr<VulkanCommandBuffer> m_CmdBuffer;
    std::unique_ptr<VulkanFrameSynchronizer> m_FrameSynchronizer;
    std::unique_ptr<VulkanVertexBuffer<Vertex2D>> m_VulkanVertexBuffer;
    std::unique_ptr<VulkanIndexBuffer> m_IndexBuffer;

    std::vector<VkDescriptorSet> m_descriptorSets;

    void createUniformBuffers();

    VkDescriptorBufferInfo m_DescriptorbBufferInfo;

    GLFWwindow* window;

    VkDebugUtilsMessengerEXT debugMessenger;
    VkSurfaceKHR surface;

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;

    VkRenderPass renderPass;
    VkCommandPool commandPool;

    VkDescriptorSetLayout m_descriptorSetLayout;

    VkFramebuffer framebuffer;

    VkCommandBuffer commandBuffer;

    uint32_t currentFrame = 0;

    VkDescriptorPool m_descriptorPool;

    std::vector<VkBuffer> m_uniformBuffers;
    std::vector<VkDeviceMemory> m_uniformBuffersMemory;
    std::vector<void*> m_uniformBuffersMapped;

    void initInstance();
    void pickSuitableDevices();
    void createSwapChain();
    void initRenderPassFactory();
    void createFramebuffers();
    void createCommandPool();
    void initCmdBufferManager();

    [[nodiscard]] VkDescriptorPool createDescriptorPool() const;

    void CreateDescriptorSets();

    void createDescriptorSetLayout();

    void createTextureImage();

    void initVulkan();
    void createSurface();
    void createGraphicsPipeline();
    void createVertexBuffer();
    void createIndexBuffer();
    void createUniformBuffer();
    void createCommandBuffer();

    void recordCommandBufferSegment(VkCommandBuffer commandBuffer, uint32_t imageIndex) const;

    void createSyncObjects();

    void updateUniformBuffer(uint32_t currentImage) const;

    void initWindow();
    void mainLoop();
    void drawFrame();
    void cleanup();
};