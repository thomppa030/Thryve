//
// Created by kprie on 14.03.2024.
//
#pragma once

#include <GLFW/glfw3.h>
#include "Core/Window.h"


class VulkanSwapChain;
namespace Thryve::Rendering {
    class VulkanContext;
}
namespace Thryve::Rendering {

    class VulkanWindow final : public Window {
    public:
        VulkanWindow(const WindowSettings& windowSpecs);
        ~VulkanWindow() override;

        void Init() override;

        [[nodiscard]] const std::string& GetTitle() const override {return m_windowTitle;};
        void SetTitle(const std::string& title) override {m_windowTitle = title;};

        [[nodiscard]] Core::SharedRef<RenderContext> GetRenderContext() const override {return m_renderContext;};

        [[nodiscard]] void* GetWindow() const override { return static_cast<GLFWwindow*>(m_window); }

        VkSurfaceKHR CreateSurface() const;

        VulkanSwapChain* GetSwapChain() const {return m_swapChain;}

    protected:
        void ShutDown() override;

        bool bFrameBufferResized{false};
    private:
        GLFWwindow *m_window;

        uint32_t m_width;
        uint32_t m_height;
        std::string m_windowTitle;

        Core::SharedRef<VulkanContext> m_renderContext;
        VulkanSwapChain* m_swapChain;

        static void FrameBufferResizeCallback(GLFWwindow* window, int width, int height);

    };

} // namespace Thryve::Rendering
