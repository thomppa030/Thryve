//
// Created by kprie on 20.03.2024.
//

#include "Core/App.h"

namespace Thryve::Core {
    App* App::s_Instance = nullptr;

    App::App()
    {
        s_Instance = this;

        m_renderContext = Rendering::RenderContext::Create();
    }
    App::~App()
    {
        m_renderContext.Reset();
    }
    void App::Run()
    {
        m_renderContext->Run();
    }
} // namespace Thryve::Core
