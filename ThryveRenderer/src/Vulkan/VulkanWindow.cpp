//
// Created by kprie on 14.03.2024.
//

#include "Vulkan/VulkanWindow.h"

#include "GLFW/glfw3.h"
#include "Vulkan/VulkanContext.h"
#include "utils/VkDebugUtils.h"

namespace Thryve::Rendering {
    VulkanWindow::VulkanWindow(const WindowSettings& windowSpecs) :
        m_window{nullptr}, m_width{windowSpecs.Width}, m_height{windowSpecs.Height}, m_windowTitle{windowSpecs.WindowTitle}
    {
    }

    VulkanWindow::~VulkanWindow()
    {
        glfwDestroyWindow(m_window);
        glfwTerminate();
    }

    void VulkanWindow::Init()
    {
        if (!glfwInit())
        {
            throw std::runtime_error("Failed to initialize GLFW.");
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        m_window = glfwCreateWindow(static_cast<int>(m_width), static_cast<int>(m_height), m_windowTitle.c_str(), nullptr, nullptr);
        if (!m_window)
        {
            glfwTerminate();
            throw std::runtime_error("Failed to create GLFW window.");
        }

        glfwSetWindowUserPointer(m_window, this);
        glfwSetFramebufferSizeCallback(m_window, FrameBufferResizeCallback);

        m_renderContext = RenderContext::Create();

        Core::SharedRef<VulkanContext> _context = m_renderContext.As<VulkanContext>();

        m_swapChain = new VulkanSwapChain(_context);
        m_swapChain->InitializeSwapChain();
    }

    VkSurfaceKHR VulkanWindow::CreateSurface() const
    {
        VkSurfaceKHR surface;
        VK_CALL(glfwCreateWindowSurface(Thryve::Rendering::VulkanContext::GetInstance(), m_window, nullptr, &surface));
        return surface;
    }

    void VulkanWindow::ShutDown()
    {
        delete m_swapChain;
    }

    void VulkanWindow::FrameBufferResizeCallback(GLFWwindow *window, int width, int height)
    {
        const auto app = static_cast<VulkanWindow *>(glfwGetWindowUserPointer(window));
        app->bFrameBufferResized = true;
    }
} // namespace myNamespace
