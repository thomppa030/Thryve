//
// Created by kprie on 17.04.2024.
//

#include "VulkanImGuiLayer.h"

#include <external/imgui/backends/imgui_impl_glfw.h>
#include <external/imgui/imgui.h>

#include "Core/App.h"

namespace Thryve::UI {
    VulkanImGuiLayer::~VulkanImGuiLayer() {}

    void VulkanImGuiLayer::OnAttach()
    {
        ImGuiLayer::OnAttach();
    }

    void VulkanImGuiLayer::OnDetach() { ImGuiLayer::OnDetach(); }

    void VulkanImGuiLayer::OnImGuiRender() { ImGuiLayer::OnImGuiRender(); }
} // namespace Thryve::UI
