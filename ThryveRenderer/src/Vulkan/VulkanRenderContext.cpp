//
// Created by kprie on 14.03.2024.
//

#include "Vulkan/VulkanRenderContext.h"


#define STB_IMAGE_IMPLEMENTATION
#define TINYOBJLOADER_IMPLEMENTATION
#include <external/imgui/backends/imgui_impl_vulkan.h>
#include <iostream>
#include "tiny_obj_loader.h"

#include "Config.h"
#include "Core/Camera.h"
#include "Core/Profiling.h"
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
    Thryve::Core::Camera g_Camera(glm::vec3(-2.0f, 10.0f, 32.0f), glm::vec3(0.0f, 5.0f, 5.0f), glm::vec3(0.0f, 0.0f, 1.0f), 0.1f, 50.0f, 16.0f/9.0f);

    VulkanRenderContext::VulkanRenderContext(): m_renderPass(nullptr),
                                                m_commandPool(nullptr){
    }

    VulkanRenderContext::~VulkanRenderContext() {
    }

    void VulkanRenderContext::CreateFramebuffers() {
        m_swapChain->CreateFramebuffers(m_device);
    }

    void VulkanRenderContext::AssignCommandPool() {
        m_commandPool = m_swapChain->GetCommandPool();
    }

    VkDescriptorPool VulkanRenderContext::CreateDescriptorPool() const {

        std::array<VkDescriptorPoolSize, 2> poolSizes{};
        // Uniform buffers
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
        // Image samplers
        poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT * 4);

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

            VkDescriptorImageInfo albedoImageInfo{};
            albedoImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            albedoImageInfo.imageView = m_AlbedoImageView;
            albedoImageInfo.sampler = m_AlbedoSampler;

            VkDescriptorImageInfo metallicImageInfo{};
            metallicImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            metallicImageInfo.imageView = m_MetallicImageView;
            metallicImageInfo.sampler = m_MetallicSampler;

            VkDescriptorImageInfo normalImageInfo{};
            normalImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            normalImageInfo.imageView = m_NormalImageView;
            normalImageInfo.sampler = m_NormalSampler;

            VkDescriptorImageInfo emmissionImageInfo{};
            emmissionImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            emmissionImageInfo.imageView = m_EmmissionImageView;
            emmissionImageInfo.sampler = m_EmmissionSampler;

            VkWriteDescriptorSet _bufferWrite = m_descriptorManager->createBufferDescriptorWrite(m_descriptorSets[i], 0, &bufferInfo);
            VkWriteDescriptorSet _albedoImageWrite = m_descriptorManager->createImageDescriptorWrite(m_descriptorSets[i], 1, &albedoImageInfo);
            VkWriteDescriptorSet _metallicImageWrite = m_descriptorManager->createImageDescriptorWrite(m_descriptorSets[i], 2, &metallicImageInfo);
            VkWriteDescriptorSet _normalImageWrite = m_descriptorManager->createImageDescriptorWrite(m_descriptorSets[i], 3, &normalImageInfo);
            VkWriteDescriptorSet _emmissionImageWrite = m_descriptorManager->createImageDescriptorWrite(m_descriptorSets[i], 4, &emmissionImageInfo);

            std::array<VkWriteDescriptorSet, 5> _descriptorWrites = {_bufferWrite, _albedoImageWrite, _metallicImageWrite, _normalImageWrite, _emmissionImageWrite};

            vkUpdateDescriptorSets(m_device, static_cast<uint32_t>(_descriptorWrites.size()), _descriptorWrites.data(), 0, nullptr);
        }
    }

    void VulkanRenderContext::CreateDescriptorSetLayout() {
        //TODO Either Refactor From DescriptorSetManager or make static, needs the Manager instantiated too early at the moment
        VulkanDescriptor _uboDescriptor(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, VK_SHADER_STAGE_VERTEX_BIT);
        VulkanDescriptor _albedoDescriptor(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,1, VK_SHADER_STAGE_FRAGMENT_BIT);
        VulkanDescriptor _metallicDescriptor(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,2, VK_SHADER_STAGE_FRAGMENT_BIT);
        VulkanDescriptor _normalDescriptor(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,3, VK_SHADER_STAGE_FRAGMENT_BIT);
        VulkanDescriptor _emmissionDescriptor(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,4, VK_SHADER_STAGE_FRAGMENT_BIT);

        m_descriptorManager->CreateDescriptorSetLayout({_uboDescriptor, _albedoDescriptor, _metallicDescriptor, _normalDescriptor, _emmissionDescriptor});
        m_descriptorSetLayout = m_descriptorManager->GetDescriptorSetLayout();
    }

    void VulkanRenderContext::CreateTextureImage(const std::string& albedoPath, const std::string& metallicPath,
                                                 const std::string& normalPath, const std::string& emmissionPath) {
        m_AlbedoTextureImage =
            std::make_unique<VulkanTextureImage>(m_swapChain->GetCommandPool(), m_commandBuffer);
        m_AlbedoTextureImage->createTextureImage(albedoPath);
        m_albedoImage = m_AlbedoTextureImage->GetTextureImage();

        m_MetallicTextureImage = std::make_unique<VulkanTextureImage>(m_swapChain->GetCommandPool(), m_commandBuffer);
        m_MetallicTextureImage->createTextureImage(metallicPath);
        m_metallicImage = m_MetallicTextureImage->GetTextureImage();

        m_NormalTextureImage = std::make_unique<VulkanTextureImage>(m_swapChain->GetCommandPool(), m_commandBuffer);
        m_NormalTextureImage->createTextureImage(normalPath);
        m_normalImage = m_NormalTextureImage->GetTextureImage();

        m_EmmissionTextureImage = std::make_unique<VulkanTextureImage>(m_swapChain->GetCommandPool(), m_commandBuffer);
        m_EmmissionTextureImage->createTextureImage(emmissionPath);
        m_EmmissionImage = m_EmmissionTextureImage->GetTextureImage();
    }

    void VulkanRenderContext::CreateTextureImageView() {
        m_AlbedoTextureImage->createTextureImageView();
        m_AlbedoImageView = m_AlbedoTextureImage->GetTextureImageView();

        m_MetallicTextureImage->createTextureImageView();
        m_MetallicImageView = m_MetallicTextureImage->GetTextureImageView();

        m_NormalTextureImage->createTextureImageView();
        m_NormalImageView = m_NormalTextureImage->GetTextureImageView();

        m_EmmissionTextureImage->createTextureImageView();
        m_EmmissionImageView = m_EmmissionTextureImage->GetTextureImageView();
    }

    void VulkanRenderContext::CreateTextureSampler() {
        m_AlbedoTextureImage->createTextureSampler();
        m_AlbedoSampler = m_AlbedoTextureImage->GetTextureSampler();

        m_MetallicTextureImage->createTextureSampler();
        m_MetallicSampler = m_MetallicTextureImage->GetTextureSampler();

        m_NormalTextureImage->createTextureSampler();
        m_NormalSampler = m_NormalTextureImage->GetTextureSampler();

        m_EmmissionTextureImage->createTextureSampler();
        m_EmmissionSampler = m_EmmissionTextureImage->GetTextureSampler();
    }

    void VulkanRenderContext::InitVulkan()
    {
        PROFILE_FUNCTION();
        PickSuitableDevices();

        m_swapChain = &Core::App::Get().GetWindow().As<VulkanWindow>()->GetSwapChain();
        m_renderPass = m_swapChain->GetRenderPass();

        m_descriptorPool = CreateDescriptorPool();
        m_descriptorManager = Core::UniqueRef<VulkanDescriptorManager>::Create(m_descriptorPool);
        CreateDescriptorSetLayout();
        CreateGraphicsPipeline();
        AssignCommandPool();
        AssignCommandBuffer();
        // Stop Refactor

        auto _albedoPath = std::string(RESOURCE_DIR) + "/Robot_Albedo_Map_1K.jpg";
        auto _metallicPath = std::string(RESOURCE_DIR) + "/Robot_Metallic_Map.jpeg";
        auto _normalPath = std::string(RESOURCE_DIR) + "/Robot_Normal_Map_1K.jpg";
        auto _emmissionPath = std::string(RESOURCE_DIR) + "/Robot_Emmission_Map.jpeg";

        CreateTextureImage(_albedoPath, _metallicPath, _normalPath, _emmissionPath);
        CreateTextureImageView();
        CreateTextureSampler();
        auto _modelPath = std::string(RESOURCE_DIR)+"/Robot_Model.obj";
        LoadModel(_modelPath);
        CreateVertexBuffer();
        CreateIndexBuffer();
        CreateUniformBuffer();
        CreateDescriptorSets();
        CreateSyncObjects();
    }
    void VulkanRenderContext::PickSuitableDevices()
    {
        PROFILE_FUNCTION();
        auto _deviceSelector = VulkanContext::GetCurrentDevice();
        m_device = _deviceSelector->GetLogicalDevice();
        m_physicalDevice = _deviceSelector->GetPhysicalDevice();
    }

    void VulkanRenderContext::MainLoop()
    {
        while (!glfwWindowShouldClose(VulkanContext::GetWindowStatic()))
        {
            glfwPollEvents();
            DrawFrame();
        }

        VK_CALL(vkDeviceWaitIdle(m_device));
    }

    void VulkanRenderContext::Cleanup() {
        PROFILE_FUNCTION();
        m_FrameSynchronizer.reset();
        m_indexBuffer.reset();
        m_vulkanVertexBuffer.reset();
        m_pipeline.reset();

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
             vkDestroyBuffer(m_device, m_uniformBuffers[i], nullptr);
             vkFreeMemory(m_device, m_uniformBuffersMemory[i], nullptr);
        }

        vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr);

        m_AlbedoTextureImage.reset();
        m_MetallicTextureImage.reset();
        m_NormalTextureImage.reset();
        m_EmmissionTextureImage.reset();

        vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayout, nullptr);
    }

    void VulkanRenderContext::CreateGraphicsPipeline() {
        PROFILE_FUNCTION();
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
        PROFILE_FUNCTION();
        auto _deviceSelector = VulkanContext::GetCurrentDevice();
        m_vulkanVertexBuffer = std::make_unique<VulkanVertexBuffer<Vertex3D>>(m_device, m_physicalDevice, m_commandPool, _deviceSelector->GetGraphicsQueue());
        std::cout << "Model Vertex Count: " << ModelVertices.size() << "\n";
        m_vulkanVertexBuffer->Create(ModelVertices);
    }

    void VulkanRenderContext::CreateIndexBuffer() {
        PROFILE_FUNCTION();
        m_indexBuffer = std::make_unique<VulkanIndexBuffer>(m_commandPool);
        std::cout << "Model Index Count: " << ModelIndices.size() << "\n";
        m_indexBuffer->Create(ModelIndices);
    }

    void VulkanRenderContext::CreateUniformBuffer() {
        PROFILE_FUNCTION();
        m_uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        m_uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
        m_uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            constexpr VkDeviceSize bufferSize = sizeof(UniformBufferObject);
            VulkanBufferUtils::CreateBuffer({ m_device, m_physicalDevice, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT}, m_uniformBuffers[i], m_uniformBuffersMemory[i]);

            VK_CALL(vkMapMemory(m_device, m_uniformBuffersMemory[i], 0, bufferSize, 0, &m_uniformBuffersMapped[i]));
        }
    }

    void VulkanRenderContext::AssignCommandBuffer() {
        m_commandBuffer = m_swapChain->GetCommandBuffer();
    }

    void VulkanRenderContext::RecordCommandBufferSegment(VkCommandBuffer commandBuffer, const uint32_t imageIndex) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    VK_CALL(vkBeginCommandBuffer(commandBuffer, &beginInfo));

    Core::App::Get().SetCurrentImageIndex(imageIndex);
    const auto framebuffers = m_swapChain->GetFrameBuffers();
    m_renderPass = m_swapChain->GetRenderPass();

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_renderPass;
    renderPassInfo.framebuffer = framebuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = m_swapChain->GetSwapchainExtent();

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {{0.02f, 0.02f, 0.02f, 1.0f}};
    clearValues[1].depthStencil = {1.f, 0};

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

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
    } else {
        m_vulkanVertexBuffer->Draw(commandBuffer);
    }

    vkCmdEndRenderPass(commandBuffer);

    VK_CALL(vkEndCommandBuffer(commandBuffer));
}

    void VulkanRenderContext::CreateSyncObjects() {
        PROFILE_FUNCTION();
        m_FrameSynchronizer = std::make_unique<VulkanFrameSynchronizer>(MAX_FRAMES_IN_FLIGHT);
    }

    void VulkanRenderContext::UpdateUniformBuffer(const uint32_t currentImage) const {
        static auto startTime = std::chrono::high_resolution_clock::now();

        const auto currentTime = std::chrono::high_resolution_clock::now();
        const float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        // Assuming you have an instance of Camera, for example, m_Camera
        UniformBufferObject ubo{};

        // Uncomment to add constant Rotation
        float rotationAngle = deltaTime * glm::radians(45.0f); // Rotate at 45 degrees per second
        ubo.model = glm::rotate(glm::mat4(1.0f), rotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));

        // Comment out if you want to add rotation
        // Set the model matrix (if you want to rotate the model over time)
        // ubo.model = glm::rotate(glm::mat4(1.0f), deltaTime * glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

        // Get the view matrix from the camera
        ubo.view = g_Camera.GetViewMatrix();

        // Get the projection matrix from the camera
        ubo.projection = g_Camera.GetProjectionMatrix();

        // Vulkan clip space has inverted Y and half Z
        ubo.projection[1][1] *= -1;

        memcpy(m_uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
    }

    /*void VulkanRenderContext::UpdateUniformBuffer(const uint32_t currentImage) const {
        static auto _startTime = std::chrono::high_resolution_clock::now();

        const auto _currentTime = std::chrono::high_resolution_clock::now();
        const float _deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(_currentTime - _startTime).count();

        UniformBufferObject _ubo{};
        _ubo.model = glm::rotate(glm::mat4(1.0f), _deltaTime * glm::radians(.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        _ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        _ubo.projection = glm::perspective(glm::radians(45.0f)
                                           , m_swapChain->GetSwapchainExtent().width / static_cast<float>(m_swapChain->
                                                                                                          GetSwapchainExtent().height), 0.1f, 10.0f);
        _ubo.projection[1][1] *= -1;

        memcpy(m_uniformBuffersMapped[currentImage], &_ubo, sizeof(_ubo));
    }*/


    void VulkanRenderContext::DrawFrame() {
        PROFILE_FUNCTION()
        auto& _syncObjects = m_FrameSynchronizer->GetSyncObjects(currentFrame);
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

                VK_CALL(vkResetFences(m_device, 1, &_syncObjects.in_flight_fence));
                m_commandBuffer = m_swapChain->GetCommandBuffer();
                // CommandBuffer goes out od scope somewhere I think
                VK_CALL(vkResetCommandBuffer(m_commandBuffer, /*VkCommandBufferResetFlagBits*/ 0));
                RecordCommandBufferSegment(m_commandBuffer, _imageIndex);

                // Core::App::Get().Run();

                if (!m_FrameSynchronizer->SubmitCommandBuffers(&m_commandBuffer, currentFrame, _imageIndex)) {
                    throw std::runtime_error("Failed to submit draw command buffer!");
                }

                //TODO Hack to get the UI drawn

                result = m_swapChain->PresentImage(_imageIndex, _syncObjects.render_finished_semaphore);
                if (m_swapChain->HandlePresentResult(result)) {
                    m_swapChain->RecreateSwapChain();
                }

                currentFrame = m_FrameSynchronizer->AdvanceFrame(currentFrame);
            }
        }
    }

    void VulkanRenderContext::Run()
    {
        InitVulkan();
        MainLoop();
        Cleanup();
    }

  void VulkanRenderContext::LoadModel(const std::string& path)
{
    PROFILE_FUNCTION()
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string err, warn;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str())) {
        throw std::runtime_error(warn + err);
    }

    for (const auto& shape : shapes) {
        for (size_t i = 0; i < shape.mesh.indices.size(); i += 3) {
            std::array<Vertex3D, 3> vertices;
            for (size_t j = 0; j < 3; j++) {
                tinyobj::index_t idx = shape.mesh.indices[i + j];

                vertices[j].pos = {
                    attrib.vertices[3 * idx.vertex_index + 0],
                    attrib.vertices[3 * idx.vertex_index + 1],
                    attrib.vertices[3 * idx.vertex_index + 2]
                };

                vertices[j].texCoord = {
                    attrib.texcoords[2 * idx.texcoord_index + 0],
                    1.0f - attrib.texcoords[2 * idx.texcoord_index + 1]
                };

                vertices[j].normal = {
                    attrib.normals[3 * idx.normal_index + 0],
                    attrib.normals[3 * idx.normal_index + 1],
                    attrib.normals[3 * idx.normal_index + 2]
                };
            }

            // Calculate tangents and bitangents
            glm::vec3 edge1 = vertices[1].pos - vertices[0].pos;
            glm::vec3 edge2 = vertices[2].pos - vertices[0].pos;
            glm::vec2 deltaUV1 = vertices[1].texCoord - vertices[0].texCoord;
            glm::vec2 deltaUV2 = vertices[2].texCoord - vertices[0].texCoord;

            float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

            glm::vec3 tangent;
            tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
            tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
            tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

            glm::vec3 bitangent;
            bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
            bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
            bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

            for (size_t j = 0; j < 3; j++) {
                vertices[j].tangent = tangent;
                vertices[j].bitangent = bitangent;

                ModelVertices.push_back(vertices[j]);
                ModelIndices.push_back(static_cast<uint32_t>(ModelVertices.size() - 1));
            }
        }
    }
}


} // namespace Thryve::Rendering
