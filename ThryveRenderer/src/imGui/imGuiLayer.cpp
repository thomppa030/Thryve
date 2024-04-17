//
// Created by kprie on 17.04.2024.
//

#include "imGuiLayer.h"

#include <external/imgui/imgui.h>

namespace Thryve::UI {

    ImGuiLayer::ImGuiLayer() : Layer{"ImGuiLayer"} {}

    void ImGuiLayer::OnAttach()
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGui::StyleColorsDark();
    }

    void ImGuiLayer::OnDetach()
    {
        ImGui::DestroyContext();
    }

    void ImGuiLayer::OnImGuiRender()
    {
        ImGui::ShowDemoWindow();
    }

    void ImGuiLayer::Begin()
    {
        ImGui::NewFrame();
    }
    void ImGuiLayer::End()
    {
        ImGuiIO& _io = ImGui::GetIO();

        ImGui::Render();
    }
} // Thryve::UI