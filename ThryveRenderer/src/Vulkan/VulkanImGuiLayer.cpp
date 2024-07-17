//
// Created by kprie on 17.04.2024.
//

#include "Vulkan/VulkanImGuiLayer.h"

#include <external/imgui/backends/imgui_impl_glfw.h>
#include <external/imgui/backends/imgui_impl_vulkan.h>
#include <external/imgui/imgui.h>

#include "Core/App.h"
#include "Vulkan/VulkanContext.h"
#include "Vulkan/VulkanSwapChain.h"
#include "Vulkan/VulkanWindow.h"
#include "utils/VkDebugUtils.h"

namespace Thryve::UI {
    static std::vector<VkCommandBuffer> s_ImGuiCommandBuffers;

    VulkanImGuiLayer::~VulkanImGuiLayer() {}

    void VulkanImGuiLayer::OnAttach()
    {
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

        ImGui::StyleColorsDark();
        //1: create descriptor pool for IMGUI
        //   the size of the pool is very oversize, but it's copied from imgui demo itself.

        constexpr int _numElements = 1000;
        const VkDescriptorPoolSize _poolSizes[] =
        {
            { VK_DESCRIPTOR_TYPE_SAMPLER, _numElements },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, _numElements },
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, _numElements },
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, _numElements },
            { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, _numElements },
            { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, _numElements },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, _numElements },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, _numElements },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, _numElements },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, _numElements },
            { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, _numElements }
        };

        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.maxSets = _numElements * static_cast<uint32_t>(IM_ARRAYSIZE(_poolSizes));
        pool_info.poolSizeCount = std::size(_poolSizes);
        pool_info.pPoolSizes = _poolSizes;

        auto _deviceSelector = Rendering::VulkanContext::GetCurrentDevice();
        m_imguiPool = std::make_unique<VulkanDescriptorPool>(_deviceSelector->GetLogicalDevice(), pool_info);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::GetIO();

        ImGui::StyleColorsDark();

        // 2: initialize imgui library
        //this initializes imgui for GLFW
        ImGui_ImplGlfw_InitForVulkan(static_cast<GLFWwindow*>(Core::App::Get().GetWindow()->GetWindow()), true);

        auto* _swapChain = &Core::App::Get().GetWindow().As<Rendering::VulkanWindow>()->GetSwapChain();
        //this initializes imgui for Vulkan
        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = Rendering::VulkanContext::GetInstance();
        init_info.PhysicalDevice = _deviceSelector->GetPhysicalDevice();
        init_info.Device = _deviceSelector->GetLogicalDevice();
        init_info.QueueFamily = _deviceSelector->GetQueueFamilyIndices().GraphicsFamily.value();
        init_info.Queue = _deviceSelector->GetGraphicsQueue();
        init_info.DescriptorPool = m_imguiPool->Get();
        init_info.RenderPass = _swapChain->GetRenderPass();
        init_info.MinImageCount = 2;
        init_info.ImageCount = _swapChain->GetImageCount();
        init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

        if (!ImGui_ImplVulkan_Init(&init_info))
        {
            vkDestroyDescriptorPool(_deviceSelector->GetLogicalDevice(), m_imguiPool->Get(), nullptr);
        };

        ImGui_ImplVulkan_CreateFontsTexture();
    }

    void VulkanImGuiLayer::OnDetach()
    {
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void VulkanImGuiLayer::OnImGuiRender()
    {
        bool _pOpen = true;
        ImGui::ShowDemoWindow(&_pOpen);
    }
    void VulkanImGuiLayer::Begin()
    {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }
    void VulkanImGuiLayer::End()
    {
        ImGui::EndFrame();
        ImGui::Render();

        auto swapChain = &Core::App::Get().GetWindow().As<Rendering::VulkanWindow>()->GetSwapChain();
        auto _deviceSelector = Rendering::VulkanContext::GetCurrentDevice();
        VulkanCommandBuffer commandBuffer(_deviceSelector->GetLogicalDevice(), swapChain->GetCommandPool());

        VkClearValue clearValues[2];
        clearValues[0].color = {{0.1f, 0.1f, 0.1f, 1.0f}};
        clearValues[1].depthStencil = {1.0f, 0};

        uint32_t width = swapChain->GetSwapchainExtent().width;
        uint32_t height = swapChain->GetSwapchainExtent().height;

        VkCommandBuffer drawCommandBuffer = commandBuffer.Get();
        VkCommandBufferBeginInfo drawCmdBufInfo = {};
        drawCmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        drawCmdBufInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        if (VK_CALL(vkBeginCommandBuffer(drawCommandBuffer, &drawCmdBufInfo)) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to begin recording command buffer!");
        }

        VkRenderPassBeginInfo renderPassBeginInfo = {};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass = swapChain->GetRenderPass();
        renderPassBeginInfo.framebuffer = swapChain->GetCurrentFramebuffer();
        renderPassBeginInfo.renderArea = {{0, 0}, {width, height}};
        renderPassBeginInfo.clearValueCount = 2;
        renderPassBeginInfo.pClearValues = clearValues;

        vkCmdBeginRenderPass(drawCommandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport = {};
        viewport.x = 0.0f;
        viewport.y = static_cast<float>(height);
        viewport.height = -static_cast<float>(height);
        viewport.width = static_cast<float>(width);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(drawCommandBuffer, 0, 1, &viewport);

        VkRect2D scissor = {};
        scissor.extent.width = width;
        scissor.extent.height = height;
        scissor.offset = {0, 0};
        vkCmdSetScissor(drawCommandBuffer, 0, 1, &scissor);

        ImDrawData* drawData = ImGui::GetDrawData();
        ImGui_ImplVulkan_RenderDrawData(drawData, drawCommandBuffer);

        vkCmdEndRenderPass(drawCommandBuffer);

        if (VK_CALL(vkEndCommandBuffer(drawCommandBuffer)) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to record command buffer!");
        }

        ImGuiIO& io = ImGui::GetIO();
        (void)io;
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }
    }
} // namespace Thryve::UI
