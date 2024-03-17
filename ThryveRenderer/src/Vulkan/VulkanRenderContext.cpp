//
// Created by kprie on 14.03.2024.
//

#include "Vulkan/VulkanRenderContext.h"


#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "glm/ext/matrix_clip_space.hpp"
#include "Vulkan/VulkanUniformBuffer.h"
#include "Config.h"
#include "Vulkan/VulkanDescriptorManager.h"
#include "Vulkan/VulkanDescriptorSetBuilder.h"
#include "Vulkan/VulkanDeviceSelector.h"
#include "utils/VulkanBufferUtils.h"
#include "utils/VkDebugUtils.h"

VulkanRenderContext::VulkanRenderContext(): window(nullptr), surface(nullptr), device(nullptr),
                                            renderPass(nullptr),
                                            commandPool(nullptr){
}

VulkanRenderContext::~VulkanRenderContext() {
}

void VulkanRenderContext::initWindow() {
    m_WindowContext = std::make_unique<VulkanWindowContext>("ThryveRenderer", WIDTH, HEIGHT);
    window = m_WindowContext->GetWindow();
}

void VulkanRenderContext::initInstance() {
    m_Instance = std::make_unique<VulkanInstance>();
    m_Instance->init("ThryveRenderer");
}

void VulkanRenderContext::pickSuitableDevices() {
    m_DeviceSelector = std::make_unique<VulkanDeviceSelector>(m_Instance->getInstance(), surface);
    m_DeviceSelector->PickSuitableDevice(deviceExtensions, enableValidationLayers);
    physicalDevice = m_DeviceSelector->GetPhysicalDevice();
    device = m_DeviceSelector->GetLogicalDevice();
}

void VulkanRenderContext::createSwapChain() {
    int width, height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    m_SwapChain->InitializeSwapChain(width, height);
}

void VulkanRenderContext::initRenderPassFactory() {
    m_RenderPassFactory = std::make_unique<VulkanRenderPassBuilder>(device);
    m_RenderPassFactory->CreateStandardRenderPasses(m_SwapChain->GetSwapchainImageFormat());
}

void VulkanRenderContext::createFramebuffers() {
    m_SwapChain->CreateFramebuffers();
}

void VulkanRenderContext::createCommandPool() {
    m_CmdPoolManager = std::make_unique<VulkanCommandPoolManager>(device, m_DeviceSelector->FindQueueFamilies(physicalDevice).graphicsFamily.value());
    commandPool = m_CmdPoolManager->GetCommandPool();
}

void VulkanRenderContext::initCmdBufferManager() {
    m_CmdBuffer = std::make_unique<VulkanCommandBuffer>(device, commandPool);
}

VkDescriptorPool VulkanRenderContext::createDescriptorPool() const {

    std::array<VkDescriptorPoolSize, 1> poolSizes{};
    // Uniform buffers
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    // Image samplers
    // poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    // poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    // The maximum number of descriptor sets that can be allocated from the pool
    poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT * 2); // Assuming one set for UB and one for samplers per frame

    VkDescriptorPool descriptorPool;
    VK_CALL(vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool));

    return descriptorPool;
}

void VulkanRenderContext::CreateDescriptorSets() {

    m_descriptorManager->allocateDescriptorSets(MAX_FRAMES_IN_FLIGHT);
    m_descriptorSets = m_descriptorManager->getDescriptorSets();

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = m_uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = m_descriptorSets[i];
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;
        descriptorWrite.pImageInfo = nullptr; // Optional
        descriptorWrite.pTexelBufferView = nullptr; // Optional
        vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
    }
}

void VulkanRenderContext::createDescriptorSetLayout() {
    //TODO Either Refactor From DescriptorSetManager or make static, needs the Manager instantiated too early at the moment
    VulkanDescriptor uboDescriptor(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, VK_SHADER_STAGE_VERTEX_BIT);
    m_descriptorManager->createDescriptorSetLayout({uboDescriptor});
    m_descriptorSetLayout = m_descriptorManager->getDescriptorSetLayout();
}

void VulkanRenderContext::createTextureImage() {
}

void VulkanRenderContext::initVulkan() {
    initInstance();
    createSurface();
    pickSuitableDevices();
    m_SwapChain = std::make_unique<VulkanSwapChain>(m_DeviceSelector.get(),surface,window);
    createSwapChain();
    initRenderPassFactory();
    renderPass = m_RenderPassFactory->GetRenderPass("default")->GetRenderPass();
    m_SwapChain->SetRenderPass(renderPass);

    m_descriptorPool = createDescriptorPool();
    m_descriptorManager = std::make_unique<VulkanDescriptorManager>(device, m_descriptorPool);
    createDescriptorSetLayout();
    createGraphicsPipeline();
    createFramebuffers();
    createCommandPool();
    createTextureImage();
    createVertexBuffer();
    createIndexBuffer();
    createUniformBuffer();
    CreateDescriptorSets();
    initCmdBufferManager();
    createCommandBuffer();
    createSyncObjects();
}

void VulkanRenderContext::mainLoop() {
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        drawFrame();
    }

    VK_CALL(vkDeviceWaitIdle(device));
}

void VulkanRenderContext::cleanup() {

    m_FrameSynchronizer.reset();
    m_CmdBuffer->Free(commandBuffer);
    m_CmdBuffer.reset();
    m_IndexBuffer.reset();
    m_VulkanVertexBuffer.reset();
    m_CmdPoolManager.reset();
    m_Pipeline.reset();
    m_RenderPassFactory.reset();
    m_SwapChain.reset();

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroyBuffer(device, m_uniformBuffers[i], nullptr);
        vkFreeMemory(device, m_uniformBuffersMemory[i], nullptr);
    }
    vkDestroyDescriptorPool(device, m_descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(device, m_descriptorSetLayout, nullptr);

    m_DeviceSelector.reset();
    vkDestroySurfaceKHR(m_Instance->getInstance(), surface, nullptr);
    m_Instance.reset();
    m_WindowContext.reset();
}

void VulkanRenderContext::createSurface() {
    surface = m_WindowContext->CreateSurface(m_Instance->getInstance());
}

void VulkanRenderContext::createGraphicsPipeline() {

    PipelineConfigInfo configInfo;
    configInfo.vertexInput.bindings = {Vertex2D::getBindingDescription()};
    configInfo.vertexInput.attributes = Vertex2D::getAttributeDescriptions();
    configInfo.SetViewportAndScissor(WIDTH, HEIGHT);
    configInfo.EnableDynamicViewportAndLineWidth();
    configInfo.descriptorSetLayout = m_descriptorSetLayout;
    configInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    configInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

    m_Pipeline = std::make_unique<VulkanPipeline>(device, renderPass);

    const auto vertexShaderPath = std::string(SHADERS_DIR)+"/SPIRV/triangle.vert.spv";
    const auto fragmentShaderPath = std::string(SHADERS_DIR)+"/SPIRV/triangle.frag.spv";

    m_Pipeline->CreatePipeline(vertexShaderPath, fragmentShaderPath, configInfo);
}

void VulkanRenderContext::createVertexBuffer() {
    m_VulkanVertexBuffer = std::make_unique<VulkanVertexBuffer<Vertex2D>>(device, physicalDevice, commandPool, m_DeviceSelector->GetGraphicsQueue());
    m_VulkanVertexBuffer->Create(vertices);
}

void VulkanRenderContext::createIndexBuffer() {
    m_IndexBuffer = std::make_unique<VulkanIndexBuffer>(device, physicalDevice, commandPool, m_DeviceSelector->GetGraphicsQueue());
    m_IndexBuffer->Create(indices);
}

void VulkanRenderContext::createUniformBuffer() {
    m_uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    m_uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
    m_uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        constexpr VkDeviceSize bufferSize = sizeof(UniformBufferObject);
        VulkanBufferUtils::createBuffer({device, physicalDevice, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT}, m_uniformBuffers[i], m_uniformBuffersMemory[i]);

        VK_CALL(vkMapMemory(device, m_uniformBuffersMemory[i], 0, bufferSize, 0, &m_uniformBuffersMapped[i]));
    }
}

void VulkanRenderContext::createCommandBuffer() {
    commandBuffer = m_CmdBuffer->Allocate();
}

void VulkanRenderContext::recordCommandBufferSegment(VkCommandBuffer commandBuffer, const uint32_t imageIndex) const {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    VK_CALL(vkBeginCommandBuffer(commandBuffer, &beginInfo));

    const auto framebuffers = m_SwapChain->GetFrameBuffers();

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = framebuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = m_SwapChain->GetSwapchainExtent();

    constexpr VkClearValue clearColor = {{{0.5f, 0.5f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline->GetPipeline());

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(m_SwapChain->GetSwapchainExtent().width);
    viewport.height = static_cast<float>(m_SwapChain->GetSwapchainExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = m_SwapChain->GetSwapchainExtent();
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    if (m_VulkanVertexBuffer) {
        m_VulkanVertexBuffer->Bind(commandBuffer);
    }

    if (m_IndexBuffer) {
        m_IndexBuffer->Bind(commandBuffer);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline->GetPipelineLayout(), 0, 1, &m_descriptorSets[currentFrame], 0, nullptr);
        m_IndexBuffer->Draw(commandBuffer);
    }
    else {
        m_VulkanVertexBuffer->Draw(commandBuffer);
    }

    vkCmdEndRenderPass(commandBuffer);

    VK_CALL(vkEndCommandBuffer(commandBuffer));
}

void VulkanRenderContext::createSyncObjects() {
    m_FrameSynchronizer = std::make_unique<VulkanFrameSynchronizer>(device, MAX_FRAMES_IN_FLIGHT, m_DeviceSelector->GetGraphicsQueue());
}

void VulkanRenderContext::updateUniformBuffer(const uint32_t currentImage) const {
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    UniformBufferObject ubo{};
    ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.projection = glm::perspective(glm::radians(45.0f)
                                      , m_SwapChain->GetSwapchainExtent().width / (float) m_SwapChain->
                                        GetSwapchainExtent().height, 0.1f, 10.0f);
    ubo.projection[1][1] *= -1;

    memcpy(m_uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}

void VulkanRenderContext::drawFrame() {
    const auto frameStart = std::chrono::steady_clock::now();

    auto& syncObjects = m_FrameSynchronizer->GetSyncObjects(currentFrame);

    if (!m_FrameSynchronizer->WaitForFences(currentFrame)) {
        VK_CALL(vkWaitForFences(device, 1, &syncObjects.inFlightFence, VK_TRUE, UINT64_MAX));
    }

    auto [result, optionalImageIndex] = m_SwapChain->AcquireNextImage(syncObjects.imageAvailableSemaphore);

    if (result == VK_SUCCESS) {
        if (optionalImageIndex.has_value()) {
            uint32_t imageIndex = optionalImageIndex.value();

            if (!m_SwapChain->HandleAcquireResult(result)) {
                m_SwapChain->RecreateSwapChain();
            }

            updateUniformBuffer(currentFrame);

            VK_CALL(vkResetFences(device, 1, &syncObjects.inFlightFence));

            VK_CALL(vkResetCommandBuffer(commandBuffer, /*VkCommandBufferResetFlagBits*/ 0));
            recordCommandBufferSegment(commandBuffer, imageIndex);

            if (!m_FrameSynchronizer->SubmitCommandBuffers(&commandBuffer, currentFrame, imageIndex)) {
                throw std::runtime_error("Failed to submit draw command buffer!");
            }

            result = m_SwapChain->PresentImage(imageIndex, syncObjects.renderFinishedSemaphore);
            if (m_SwapChain->HandlePresentResult(result)) {
                m_SwapChain->RecreateSwapChain();
            }

            currentFrame = m_FrameSynchronizer->AdvanceFrame(currentFrame);
            const auto frameEnd = std::chrono::steady_clock::now();
            const std::chrono::duration<float, std::milli> frametime = frameEnd - frameStart;
            const float msPerFrame = frametime.count();
        }
    }
}

void VulkanRenderContext::run() {
    initWindow();
    initVulkan();
    mainLoop();
    cleanup();
}