//
// Created by kprie on 14.03.2024.
//
#pragma once

#include <GLFW/glfw3.h>

#include "Core/Ref.h"


class VulkanWindowContext : public Thryve::Core::ReferenceCounted {
public:
    VulkanWindowContext(const char* windowTitle, int width, int height);
    ~VulkanWindowContext();

    [[nodiscard]] GLFWwindow *GetWindow() const { return m_window; }
    VkSurfaceKHR CreateSurface(VkInstance instance) const;

    bool bFrameBufferResized{false};

private:
    GLFWwindow* m_window;
    int m_width;
    int m_height;
    const char* m_windowTitle;

    static void FrameBufferResizeCallback(GLFWwindow* window, int width, int height);
};
