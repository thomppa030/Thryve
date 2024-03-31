//
// Created by kprie on 20.03.2024.
//

#include "Core/App.h"
#include "Core/System.h"

namespace Thryve::Core {
    App* App::s_Instance = nullptr;
    AppSpecification App::s_AppSpecification = {};

    App::App()
    {
        s_Instance = this;

        PopulateAppSpecs();
        m_renderContext = Rendering::RenderContext::Create();
    }
    App::~App() { m_renderContext.Reset(); }

    void App::PopulateAppSpecs()
    {
        s_AppSpecification.OSName = SystemSpecs::GetOSName();
        s_AppSpecification.CPU = SystemSpecs::GetCPUName();
        s_AppSpecification.CPUCoreCount = SystemSpecs::GetProcessorCoreCount();
        s_AppSpecification.RAM = SystemSpecs::GetTotalRAM();
    }

    AppSpecification App::GetAppSpecification() { return s_AppSpecification; }

    void App::Run()
    {
        m_renderContext->Run();
    }
} // namespace Thryve::Core
