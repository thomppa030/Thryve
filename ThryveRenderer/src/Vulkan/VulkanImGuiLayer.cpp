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
        // the size of the pool is very oversize, but it's copied from imgui demo itself.
        VkDescriptorPoolSize pool_sizes[] =
        {
            { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
        };

        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.maxSets = 1000;
        pool_info.poolSizeCount = std::size(pool_sizes);
        pool_info.pPoolSizes = pool_sizes;

        auto _deviceSelector = Rendering::VulkanContext::GetCurrentDevice();
        VkDescriptorPool imguiPool;
        VK_CALL(vkCreateDescriptorPool(_deviceSelector->GetLogicalDevice(), &pool_info, nullptr, &imguiPool));


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
        init_info.DescriptorPool = imguiPool;
        init_info.RenderPass = _swapChain->GetRenderPass();
        init_info.MinImageCount = 2;
        init_info.ImageCount = _swapChain->GetImageCount();
        init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

        ImGui_ImplVulkan_Init(&init_info);

        ImGui_ImplVulkan_CreateFontsTexture();

        VK_CALL(vkDeviceWaitIdle(_deviceSelector->GetLogicalDevice()));
        ImGui_ImplVulkan_DestroyFontsTexture();
    }

    void VulkanImGuiLayer::OnDetach()
    {
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
    }

    void VulkanImGuiLayer::OnImGuiRender()
    {
        ImGui::Begin("Main Workspace");
        ImGui::DockSpace(ImGui::GetID("MyDockSpace"), ImVec2(2160,1440), ImGuiDockNodeFlags_None);
        ImGui::SetNextWindowDockID(ImGui::GetID("MyDockSpace"), ImGuiCond_FirstUseEver);
        ImGui::Begin("Viewport");
        ImGui::SetNextWindowDockID(ImGui::GetID("MyDockSpace"), ImGuiCond_FirstUseEver);
        ImGui::Begin("Scene Hierarchy");
        // Contents of Scene Hierarchy
        ImGui::End();

        ImGui::SetNextWindowDockID(ImGui::GetID("MyDockSpace"), ImGuiCond_FirstUseEver);
        ImGui::Begin("Properties");
        // Contents of Properties Panel
        ImGui::End();
        ImGui::End(); // End for Viewport
        ImGui::End(); // End for Main Workspace

        bool p_open = true;
        ImGui::ShowDemoWindow(&p_open);
    }
    void VulkanImGuiLayer::Begin()
    {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }
    void VulkanImGuiLayer::End()
    {
        ImGui::Render();

        auto swapChain = &Core::App::Get().GetWindow().As<Rendering::VulkanWindow>()->GetSwapChain();

		VkClearValue clearValues[2];
		clearValues[0].color = { {0.1f, 0.1f,0.1f, 1.0f} };
		clearValues[1].depthStencil = { 1.0f, 0 };

		uint32_t width = swapChain->GetSwapchainExtent().width;
		uint32_t height = swapChain->GetSwapchainExtent().height;

		VkCommandBufferBeginInfo drawCmdBufInfo = {};
		drawCmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		drawCmdBufInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		drawCmdBufInfo.pNext = nullptr;

        // TODO Needs to be Primamry and Secondary Commandbuffer instead of just one!
        // This needs a bigger change to the overall Rendering of the Engine, possibly 2 separate renderthreads!
		VkCommandBuffer drawCommandBuffer = swapChain->GetCommandBuffer();
		VK_CALL(vkBeginCommandBuffer(drawCommandBuffer, &drawCmdBufInfo));

		VkRenderPassBeginInfo renderPassBeginInfo = {};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.pNext = nullptr;
		renderPassBeginInfo.renderPass = swapChain->GetRenderPass();
		renderPassBeginInfo.renderArea.offset.x = 0;
		renderPassBeginInfo.renderArea.offset.y = 0;
		renderPassBeginInfo.renderArea.extent.width = width;
		renderPassBeginInfo.renderArea.extent.height = height;
		renderPassBeginInfo.clearValueCount = 2; // Color + depth
		renderPassBeginInfo.pClearValues = clearValues;
		renderPassBeginInfo.framebuffer = swapChain->GetCurrentFramebuffer();

		vkCmdBeginRenderPass(drawCommandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkCommandBufferInheritanceInfo _inheritanceInfo = {};
		_inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
		_inheritanceInfo.renderPass = swapChain->GetRenderPass();
		_inheritanceInfo.framebuffer = swapChain->GetCurrentFramebuffer();

		VkCommandBufferBeginInfo _cmdBufInfo = {};
		_cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		_cmdBufInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
		_cmdBufInfo.pInheritanceInfo = &_inheritanceInfo;

		VkViewport _viewport = {};
		_viewport.x = 0.0f;
		_viewport.y = static_cast<float>(height);
		_viewport.height = -static_cast<float>(height);
		_viewport.width = static_cast<float>(width);
		_viewport.minDepth = 0.0f;
		_viewport.maxDepth = 1.0f;
		vkCmdSetViewport(drawCommandBuffer, 0, 1, &_viewport);

		VkRect2D _scissor = {};
		_scissor.extent.width = width;
		_scissor.extent.height = height;
		_scissor.offset.x = 0;
		_scissor.offset.y = 0;
		vkCmdSetScissor(drawCommandBuffer, 0, 1, &_scissor);

		ImDrawData* _mainDrawData = ImGui::GetDrawData();
		ImGui_ImplVulkan_RenderDrawData(_mainDrawData, drawCommandBuffer);

		vkCmdEndRenderPass(drawCommandBuffer);

		VK_CALL(vkEndCommandBuffer(drawCommandBuffer));

        ImGuiIO& io = ImGui::GetIO(); (void)io;
        // Update and Render additional Platform Windows
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }
    }
} // namespace Thryve::UI
