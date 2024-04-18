//
// Created by kprie on 17.04.2024.
//
#pragma once
#include "../imGui/imGuiLayer.h"

namespace Thryve::UI {

class VulkanImGuiLayer final : public ImGuiLayer {
    public:

        // TODO Implement and utilize!
        ~VulkanImGuiLayer() override;
        void OnAttach() override;
        void OnDetach() override;
        void OnImGuiRender() override;

        void Begin() override;
        void End() override;
};
} // UI
// Thryve
