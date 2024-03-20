//
// Created by kprie on 20.03.2024.
//
#pragma once

#include "Ref.h"
#include "../../../IRenderContext.h"

namespace Thryve {

    class App {
    public:
        static App& Get() { return *s_Instance; }

        [[nodiscard]] Core::SharedRef<IRenderContext> GetRenderContext() const { return m_renderContext; };

    private:
        static App* s_Instance;
        Core::SharedRef<IRenderContext> m_renderContext{nullptr};
    };
} // Thryve