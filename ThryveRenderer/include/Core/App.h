//
// Created by kprie on 20.03.2024.
//
#pragma once

#include "Ref.h"
#include "Renderer/RenderContext.h"

namespace Thryve::Core {
    class App {
    public:
        App();
        static App& Get() { return *s_Instance; }

        [[nodiscard]] Core::SharedRef<Rendering::RenderContext> GetRenderContext() const { return m_renderContext; };

    private:
        static App* s_Instance;
        Core::SharedRef<Rendering::RenderContext> m_renderContext{nullptr};
    };
} // Thryve
