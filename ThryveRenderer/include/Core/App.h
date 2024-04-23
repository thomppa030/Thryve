//
// Created by kprie on 20.03.2024.
//
#pragma once

#include "LayerStack.h"
#include "Ref.h"
#include "Renderer/RenderContext.h"
#include "Window.h"

namespace Thryve::UI {
    class ImGuiLayer;
}
struct AppSpecification {
    std::string OSName;
    std::string CPU;
    int CPUCoreCount;
    std::string GPU;
    std::string GPUType;
    std::string RAM;
};

namespace Thryve::Core {
    class App {

    public:
        App();
        ~App();

        void SetCurrentImageIndex(const uint32_t index) {CurrentImageIndex = index;};

        uint32_t GetCurrentImageIndex() const {return CurrentImageIndex;}

        static App& Get() { return *s_Instance; }

        static AppSpecification GetAppSpecification();

        static void SetGraphicsCardName(const std::string& GPUName) {s_AppSpecification.GPU = GPUName;}
        static void SetGraphicsCardType(const std::string& GPUType) {s_AppSpecification.GPUType = GPUType;}

        SharedRef<Rendering::Window> GetWindow() const {return m_window;}

        void PushLayer(Layer* layer);
        void PushOverlay(Layer* overlay);

        [[nodiscard]] Core::SharedRef<Rendering::RenderContext> GetRenderContext() const { return m_renderContext; };

        void Run();

    private:
        static App* s_Instance;
        static AppSpecification s_AppSpecification;

        SharedRef<Rendering::Window> m_window;

        void PopulateAppSpecs();

        UI::ImGuiLayer* m_imGuiLayer;
        LayerStack m_layerStack;

        uint32_t CurrentImageIndex{0};

        SharedRef<Rendering::RenderContext> m_renderContext{nullptr};
    };
} // Thryve

