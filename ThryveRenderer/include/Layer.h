//
// Created by kprie on 17.04.2024.
//
#pragma once

#include "pch.h"

namespace Thryve {

    class Layer {
    public:
        explicit Layer(std::string  layerName = "Layer");
        virtual ~Layer() = default;

        virtual void OnAttach() {};
        virtual void OnDetach() {};

        // TODO will need Event and Frametime logic, but for now it serves it's purpose
        // virtual void OnEvent(Event& event) {};
        // virtual void OnUpdate(FrameTime ft) {};
        virtual void OnImGuiRender() {};

        [[nodiscard]] const std::string& GetName() const {return m_LayerName;}
    private:
        std::string m_LayerName;
    };

} // namespace Thryve::Core
