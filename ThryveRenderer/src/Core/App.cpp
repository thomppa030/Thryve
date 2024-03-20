//
// Created by kprie on 20.03.2024.
//

#include "Core/App.h"

namespace Thryve::Core {
    Core::App* Core::App::s_Instance = nullptr;

    App::App() {
        s_Instance = this;

        m_renderContext = Rendering::RenderContext::Create();
    }
}
