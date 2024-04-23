//
// Created by kprie on 17.04.2024.
//


#include "imGui/imGuiLayer.h"


#include <external/imgui/backends/imgui_impl_glfw.h>
#include <external/imgui/backends/imgui_impl_vulkan.h>
#include <external/imgui/imgui.h>

#include "Core/App.h"
#include "Core/GraphicsAPI.h"
#include "Vulkan/VulkanImGuiLayer.h"

namespace Thryve::UI {

    ImGuiLayer::ImGuiLayer() : Layer{"ImGuiLayer"} {}

    ImGuiLayer* ImGuiLayer::Create()
    {
        switch (Core::RenderAPI::CurrentAPI())
        {
        case Core::GraphicsAPIType::NONE:
            return nullptr;
        case Core::GraphicsAPIType::VULKAN:
            return new VulkanImGuiLayer;
        }
        return nullptr;
    }
} // Thryve::UI