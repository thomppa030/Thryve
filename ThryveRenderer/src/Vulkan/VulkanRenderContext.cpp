//
// Created by kprie on 14.03.2024.
//

#include "Vulkan/VulkanRenderContext.h"


#define STB_IMAGE_IMPLEMENTATION
#include <iostream>

#include "Config.h"
#include "Core/Log.h"
#include "Core/ServiceRegistry.h"
#include "Vulkan/VulkanContext.h"
#include "Vulkan/VulkanDescriptorManager.h"
#include "Vulkan/VulkanDescriptorSetBuilder.h"
#include "Vulkan/VulkanDeviceSelector.h"
#include "Vulkan/VulkanUniformBuffer.h"
#include "glm/ext/matrix_clip_space.hpp"
#include "stb_image.h"
#include "utils/ImageUtils.h"
#include "utils/VkDebugUtils.h"
#include "utils/VulkanBufferUtils.h"

namespace Thryve::Rendering {
    VulkanRenderContext::VulkanRenderContext(): m_renderPass(nullptr),
                                                m_commandPool(nullptr){
    }

    VulkanRenderContext::~VulkanRenderContext() {
    }

    void VulkanRenderContext::CreateSwapChain() {
        int width, height = 0;
        glfwGetFramebufferSize(VulkanContext::GetWindow(), &width, &height);
        m_swapChain->InitializeSwapChain(width, height);
    }

    void VulkanRenderContext::InitRenderPassFactory() {
        m_renderPassFactory = std::make_unique<VulkanRenderPassBuilder>();
        m_renderPassFactory->CreateStandardRenderPasses(m_swapChain->GetSwapchainImageFormat());
    }

    void VulkanRenderContext::CreateFramebuffers() {
        m_swapChain->CreateFramebuffers();
    }

    void VulkanRenderContext::CreateCommandPool() {
        m_cmdPoolManager = std::make_unique<VulkanCommandPoolManager>();
        m_commandPool = m_cmdPoolManager->GetCommandPool();
    }

    void VulkanRenderContext::InitCmdBufferManager() {
        m_cmdBuffer = std::make_unique<VulkanCommandBuffer>(m_commandPool);
    }

    VkDescriptorPool VulkanRenderContext::CreateDescriptorPool() const {

        std::array<VkDescriptorPoolSize, 2> poolSizes{};
        // Uniform buffers
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
        // Image samplers
        poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        // The maximum number of descriptor sets that can be allocated from the pool
        poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT * 2); // Assuming one set for UB and one for samplers per frame

        VkDescriptorPool descriptorPool;
        VK_CALL(vkCreateDescriptorPool(VulkanContext::GetCurrentDevice()->GetLogicalDevice(), &poolInfo, nullptr, &descriptorPool));

        return descriptorPool;
    }

    void VulkanRenderContext::CreateDescriptorSets() {

        m_descriptorManager->AllocateDescriptorSets(MAX_FRAMES_IN_FLIGHT);
        m_descriptorSets = m_descriptorManager->GetDescriptorSets();

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = m_uniformBuffers[i];
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UniformBufferObject);

            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = m_textureImageView;
            imageInfo.sampler = m_textureSampler;

            std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = m_descriptorSets[i];
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo = &bufferInfo;
            descriptorWrites[0].pTexelBufferView = nullptr; // Optional

            descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[1].dstSet = m_descriptorSets[i];
            descriptorWrites[1].dstBinding = 1;
            descriptorWrites[1].dstArrayElement = 0;
            descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[1].descriptorCount = 1;
            descriptorWrites[1].pImageInfo = &imageInfo; // Optional
            descriptorWrites[1].pTexelBufferView = nullptr; // Optional

            vkUpdateDescriptorSets(VulkanContext::GetCurrentDevice()->GetLogicalDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        }
    }

    void VulkanRenderContext::CreateDescriptorSetLayout() {
        //TODO Either Refactor From DescriptorSetManager or make static, needs the Manager instantiated too early at the moment
        VulkanDescriptor uboDescriptor(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, VK_SHADER_STAGE_VERTEX_BIT);
        VulkanDescriptor samplerDescriptor(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
        m_descriptorManager->CreateDescriptorSetLayout({uboDescriptor, samplerDescriptor});
        m_descriptorSetLayout = m_descriptorManager->GetDescriptorSetLayout();
    }

    void VulkanRenderContext::CreateTextureImage() {
        m_VulkanTextureImage = std::make_unique<VulkanTextureImage>(m_cmdPoolManager->GetCommandPool(), m_commandBuffer);
        m_VulkanTextureImage->createTextureImage(std::string(RESOURCE_DIR)+"/statue.jpg");
        m_textureImage = m_VulkanTextureImage->GetTextureImage();
    }

    void VulkanRenderContext::CreateTextureImageView() {
        m_VulkanTextureImage->createTextureImageView();
        m_textureImageView = m_VulkanTextureImage->GetTextureImageView();
    }

    void VulkanRenderContext::CreateTextureSampler() {
        m_VulkanTextureImage->createTextureSampler();
        m_textureSampler = m_VulkanTextureImage->GetTextureSampler();
    }

    void VulkanRenderContext::InitVulkan()
    {
        PickSuitableDevices();
        m_swapChain = std::make_unique<VulkanSwapChain>();
        CreateSwapChain();
        InitRenderPassFactory();
        m_renderPass = m_renderPassFactory->GetRenderPass("default")->GetRenderPass();
        m_swapChain->SetRenderPass(m_renderPass);

        m_descriptorPool = CreateDescriptorPool();
        m_descriptorManager = std::make_unique<VulkanDescriptorManager>(m_descriptorPool);
        CreateDescriptorSetLayout();
        CreateGraphicsPipeline();
        CreateFramebuffers();
        CreateCommandPool();
        CreateTextureImage();
        CreateTextureImageView();
        CreateTextureSampler();
        CreateVertexBuffer();
        CreateIndexBuffer();
        CreateUniformBuffer();
        CreateDescriptorSets();
        InitCmdBufferManager();
        CreateCommandBuffer();
        CreateSyncObjects();
    }
    void VulkanRenderContext::PickSuitableDevices()
    {
        auto _deviceSelector = VulkanContext::GetCurrentDevice();
        m_device = _deviceSelector->GetLogicalDevice();
        m_physicalDevice = _deviceSelector->GetPhysicalDevice();
    }

    void VulkanRenderContext::MainLoop() {
        while (!glfwWindowShouldClose(VulkanContext::GetWindow())) {
            glfwPollEvents();
            DrawFrame();
        }

        VK_CALL(vkDeviceWaitIdle(m_device));
    }

    void VulkanRenderContext::Cleanup() {

        m_FrameSynchronizer.reset();
        m_cmdBuffer->Free(m_commandBuffer);
        m_cmdBuffer.reset();
        m_indexBuffer.reset();
        m_vulkanVertexBuffer.reset();
        m_cmdPoolManager.reset();
        m_pipeline.reset();
        m_renderPassFactory.reset();
        m_swapChain.reset();

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
             vkDestroyBuffer(m_device, m_uniformBuffers[i], nullptr);
             vkFreeMemory(m_device, m_uniformBuffersMemory[i], nullptr);
        }

        vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr);

        m_VulkanTextureImage.reset();

        vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayout, nullptr);
    }

    void VulkanRenderContext::CreateGraphicsPipeline() {

        PipelineConfigInfo configInfo;
        configInfo.vertexInput.bindings = {Vertex3D::getBindingDescription()};
        configInfo.vertexInput.attributes = Vertex3D::getAttributeDescriptions();
        configInfo.SetViewportAndScissor(WIDTH, HEIGHT);
        configInfo.EnableDynamicViewportAndLineWidth();
        configInfo.descriptorSetLayout = m_descriptorSetLayout;
        configInfo.cullMode = VK_CULL_MODE_BACK_BIT;
        configInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

        m_pipeline = std::make_unique<VulkanPipeline>(m_renderPass);

        const auto vertexShaderPath = std::string(SHADERS_DIR)+"/SPIRV/triangle.vert.spv";
        const auto fragmentShaderPath = std::string(SHADERS_DIR)+"/SPIRV/triangle.frag.spv";

        m_pipeline->CreatePipeline(vertexShaderPath, fragmentShaderPath, configInfo);
    }

    void VulkanRenderContext::CreateVertexBuffer() {
        auto _deviceSelector = VulkanContext::GetCurrentDevice();
        m_vulkanVertexBuffer = std::make_unique<VulkanVertexBuffer<Vertex3D>>(m_device, m_physicalDevice, m_commandPool, _deviceSelector->GetGraphicsQueue());
        m_vulkanVertexBuffer->Create(VERTICES_3D);
    }

    void VulkanRenderContext::CreateIndexBuffer() {
        m_indexBuffer = std::make_unique<VulkanIndexBuffer>(m_commandPool);
        m_indexBuffer->Create(INDICES_3D);
    }

    void VulkanRenderContext::CreateUniformBuffer() {
        m_uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        m_uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
        m_uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            constexpr VkDeviceSize bufferSize = sizeof(UniformBufferObject);
            auto _deviceSelector = VulkanContext::GetCurrentDevice();
            VulkanBufferUtils::CreateBuffer({ _deviceSelector->GetLogicalDevice(), _deviceSelector->GetPhysicalDevice(), bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT}, m_uniformBuffers[i], m_uniformBuffersMemory[i]);

            VK_CALL(vkMapMemory(_deviceSelector->GetLogicalDevice(), m_uniformBuffersMemory[i], 0, bufferSize, 0, &m_uniformBuffersMapped[i]));
        }
    }

    void VulkanRenderContext::CreateCommandBuffer() {
        m_commandBuffer = m_cmdBuffer->Allocate();
    }

    void VulkanRenderContext::RecordCommandBufferSegment(VkCommandBuffer commandBuffer, const uint32_t imageIndex) const {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        VK_CALL(vkBeginCommandBuffer(commandBuffer, &beginInfo));

        const auto framebuffers = m_swapChain->GetFrameBuffers();

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_renderPass;
        renderPassInfo.framebuffer = framebuffers[imageIndex];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = m_swapChain->GetSwapchainExtent();

        constexpr VkClearValue clearColor = {{{0.5f, 0.7f, 0.5f, 1.0f}}};
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline->GetPipeline());

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(m_swapChain->GetSwapchainExtent().width);
        viewport.height = static_cast<float>(m_swapChain->GetSwapchainExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = m_swapChain->GetSwapchainExtent();
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        if (m_vulkanVertexBuffer) {
            m_vulkanVertexBuffer->Bind(commandBuffer);
        }

        if (m_indexBuffer) {
            m_indexBuffer->Bind(commandBuffer);
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline->GetPipelineLayout(), 0, 1, &m_descriptorSets[currentFrame], 0, nullptr);
            m_indexBuffer->Draw(commandBuffer);
        }
        else {
            m_vulkanVertexBuffer->Draw(commandBuffer);
        }

        vkCmdEndRenderPass(commandBuffer);

        VK_CALL(vkEndCommandBuffer(commandBuffer));
    }

    void VulkanRenderContext::CreateSyncObjects() {
        m_FrameSynchronizer = std::make_unique<VulkanFrameSynchronizer>(MAX_FRAMES_IN_FLIGHT);
    }

    void VulkanRenderContext::UpdateUniformBuffer(const uint32_t currentImage) const {
        static auto _startTime = std::chrono::high_resolution_clock::now();

        const auto _currentTime = std::chrono::high_resolution_clock::now();
        const float _deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(_currentTime - _startTime).count();

        UniformBufferObject _ubo{};
        _ubo.model = glm::rotate(glm::mat4(1.0f), _deltaTime * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        _ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        _ubo.projection = glm::perspective(glm::radians(45.0f)
                                           , m_swapChain->GetSwapchainExtent().width / static_cast<float>(m_swapChain->
                                                                                                          GetSwapchainExtent().height), 0.1f, 10.0f);
        _ubo.projection[1][1] *= -1;

        memcpy(m_uniformBuffersMapped[currentImage], &_ubo, sizeof(_ubo));
    }

    void VulkanRenderContext::DrawFrame() {
        auto& _syncObjects = m_FrameSynchronizer->GetSyncObjects(currentFrame);
        auto frameStart = std::chrono::high_resolution_clock::now();
        if (!m_FrameSynchronizer->WaitForFences(currentFrame)) {
            VK_CALL(vkWaitForFences(m_device, 1, &_syncObjects.in_flight_fence, VK_TRUE, UINT64_MAX));
        }

        if (auto [result, optionalImageIndex] = m_swapChain->AcquireNextImage(_syncObjects.image_available_semaphore);
            result == VK_SUCCESS) {
            if (optionalImageIndex.has_value()) {
                const uint32_t _imageIndex = optionalImageIndex.value();

                if (!m_swapChain->HandleAcquireResult(result)) {
                    m_swapChain->RecreateSwapChain();
                }

                UpdateUniformBuffer(currentFrame);

                VK_CALL(vkResetFences(VulkanContext::GetCurrentDevice()->GetLogicalDevice(), 1, &_syncObjects.in_flight_fence));

                VK_CALL(vkResetCommandBuffer(m_commandBuffer, /*VkCommandBufferResetFlagBits*/ 0));
                RecordCommandBufferSegment(m_commandBuffer, _imageIndex);

                if (!m_FrameSynchronizer->SubmitCommandBuffers(&m_commandBuffer, currentFrame, _imageIndex)) {
                    throw std::runtime_error("Failed to submit draw command buffer!");
                }

                result = m_swapChain->PresentImage(_imageIndex, _syncObjects.render_finished_semaphore);
                if (m_swapChain->HandlePresentResult(result)) {
                    m_swapChain->RecreateSwapChain();
                }

                currentFrame = m_FrameSynchronizer->AdvanceFrame(currentFrame);
            }
        }
        auto FrameEnd = std::chrono::high_resolution_clock::now();
        auto FramTimeInMs = std::chrono::duration_cast<std::chrono::microseconds>(FrameEnd - frameStart).count() / 1000.0f;

        std::cout << FramTimeInMs << "\n";
    }

    void VulkanRenderContext::Run()
    {
        InitVulkan();
        MainLoop();
        Cleanup();
    }
    void VulkanRenderContext::createDepthResources()
    {
        VkFormat _depthFormat = ImageUtils::FindDepthFormat();
        ImageUtils::createImage(m_swapChain->GetSwapchainExtent().width, m_swapChain->GetSwapchainExtent().height,
                                _depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, dethImageMemory);
        ImageUtils::CreateImageView(depthImage, depthimageView, _depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
        ImageUtils::transitionImageLayout(
            m_device, m_commandPool, VulkanContext::GetCurrentDevice()->GetGraphicsQueue(), depthImage, _depthFormat,
            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    }

} // namespace Thryve::Rendering
