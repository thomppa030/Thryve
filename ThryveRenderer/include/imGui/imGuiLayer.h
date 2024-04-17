//
// Created by kprie on 17.04.2024.
//
#pragma once
#include "Layer.h"

namespace Thryve::UI {

    class ImGuiLayer : public Layer {
    public:
        ImGuiLayer();
        ~ImGuiLayer() override = default;

        void OnAttach() override;
        void OnDetach() override;
        void OnImGuiRender() override;

        void Begin();
        void End();

    private:
        float m_Time{0.0f};
    };

} // namespace Thryve::UI