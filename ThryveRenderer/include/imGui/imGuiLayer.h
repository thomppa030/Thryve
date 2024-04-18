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

        static ImGuiLayer* Create();

        virtual void Begin() = 0;
        virtual void End() = 0;

    private:
        float m_Time{0.0f};
    };
} // namespace Thryve::UI