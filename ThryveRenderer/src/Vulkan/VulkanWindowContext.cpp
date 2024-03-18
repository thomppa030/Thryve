//
// Created by kprie on 14.03.2024.
//

#include "Vulkan/VulkanWindowContext.h"

#include "GLFW/glfw3.h"
#include "utils/VkDebugUtils.h"

VulkanWindowContext::VulkanWindowContext(const char *windowTitle, const int width, const int height): m_window(nullptr),
                                                                                                      m_width(width),
                                                                                                      m_height(height),
                                                                                                      m_windowTitle(windowTitle) {
    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW.");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    m_window = glfwCreateWindow(m_width, m_height, m_windowTitle, nullptr, nullptr);
    if (!m_window) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window.");
    }

    glfwSetWindowUserPointer(m_window, this);
    glfwSetFramebufferSizeCallback(m_window, VulkanWindowContext::FrameBufferResizeCallback);
}

VulkanWindowContext::~VulkanWindowContext() {
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

VkSurfaceKHR VulkanWindowContext::CreateSurface(VkInstance instance) const {
    VkSurfaceKHR surface;
    VK_CALL(glfwCreateWindowSurface(instance, m_window, nullptr, &surface));
    return surface;
}

void VulkanWindowContext::FrameBufferResizeCallback(GLFWwindow *window, int width, int height) {
    const auto app = static_cast<VulkanWindowContext*>(glfwGetWindowUserPointer(window));
        app->bFrameBufferResized = true;
}
