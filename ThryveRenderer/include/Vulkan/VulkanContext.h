//
// Created by kprie on 20.03.2024.
//
#pragma once

#include "Renderer/RenderContext.h"
#include "Renderer/Renderer.h"
#include "Vulkan/VulkanInstance.h"
#include "Vulkan/VulkanRenderContext.h"
#include "Vulkan/VulkanWindowContext.h"
#include "VulkanDeviceSelector.h"

const std::vector<const char *> DEVICE_EXTENSIONS = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

#ifdef NDEBUG
constexpr bool ENABLE_VALIDATION_LAYERS = false;
#else
constexpr bool ENABLE_VALIDATION_LAYERS = true;
#endif

class VulkanRenderContext;

namespace Thryve::Rendering {
    class VulkanContext final : public RenderContext {
    public:
        VulkanContext();
        ~VulkanContext() override;

        [[nodiscard]] Core::SharedRef<VulkanDeviceSelector> GetDevice() const {return m_device;}

        static GLFWwindow* GetWindow() {return s_Window;}
        static VkInstance GetInstance() {return s_Instance;}
        static VkSurfaceKHR GetSurface() {return s_Surface;}

        static Core::SharedRef<VulkanContext> Get() {
            return static_cast<Core::SharedRef<VulkanContext>>(Renderer::GetContext());
        }
        static Core::SharedRef<VulkanDeviceSelector> GetCurrentDevice() {
            return Get()->GetDevice();
        }

        void Init() override;
        void Run() override;

    private:
        Core::SharedRef<VulkanWindowContext> m_windowContext;
        Core::SharedRef<VulkanDeviceSelector> m_device;
        Core::SharedRef<VulkanInstance> m_vulkanInstance;
        Core::SharedRef<VulkanRenderContext> m_renderContext;

        inline static VkInstance s_Instance;
        inline static VkSurfaceKHR s_Surface;
        inline static GLFWwindow* s_Window;
    };

}
