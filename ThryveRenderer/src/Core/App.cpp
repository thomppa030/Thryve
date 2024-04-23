//
// Created by kprie on 20.03.2024.
//

#include "Core/App.h"

#include "Core/System.h"
#include "Core/Window.h"
#include "Layer.h"
#include "imGui/imGuiLayer.h"

namespace Thryve::Core {
    App* App::s_Instance = nullptr;
    AppSpecification App::s_AppSpecification = {};

    App::App()
    {
        s_Instance = this;

        PopulateAppSpecs();
        m_window = Rendering::Window::Create(Rendering::WindowSettings());
        m_window->Init();
        m_renderContext = m_window->GetRenderContext();
        m_imGuiLayer = UI::ImGuiLayer::Create();
        // We also Attach the Layer here
        PushLayer(m_imGuiLayer);
    }
    App::~App()
    {
    }

    void App::PopulateAppSpecs()
    {
        s_AppSpecification.OSName = SystemSpecs::GetOSName();
        s_AppSpecification.CPU = SystemSpecs::GetCPUName();
        s_AppSpecification.CPUCoreCount = SystemSpecs::GetProcessorCoreCount();
        s_AppSpecification.RAM = SystemSpecs::GetTotalRAM();
    }

    AppSpecification App::GetAppSpecification() { return s_AppSpecification; }

    void App::PushLayer(Layer* layer)
    {
        m_layerStack.PushLayer(layer);
    }

    void App::PushOverlay(Layer* overlay)
    {
        m_layerStack.PushOverlay(overlay);
        overlay->OnAttach();
    }

    void App::Run()
    {
        m_imGuiLayer->Begin();
        for (auto* layer : m_layerStack)
        {
              layer->OnImGuiRender();
        }
        m_imGuiLayer->End();

        m_renderContext->Run();
    }
} // namespace Thryve::Core
