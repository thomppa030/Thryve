//
// Created by kprie on 20.03.2024.
//
#pragma once

#include "Ref.h"
#include "Renderer/RenderContext.h"

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
        static App& Get() { return *s_Instance; }

        static AppSpecification GetAppSpecification();

        static void SetGraphicsCardName(const std::string& GPUName) {s_AppSpecification.GPU = GPUName;}
        static void SetGraphicsCardType(const std::string& GPUType) {s_AppSpecification.GPUType = GPUType;}

        [[nodiscard]] Core::SharedRef<Rendering::RenderContext> GetRenderContext() const { return m_renderContext; };

        void Run();

    private:
        static App* s_Instance;
        static AppSpecification s_AppSpecification;

        void PopulateAppSpecs();

        SharedRef<Rendering::RenderContext> m_renderContext{nullptr};
    };
} // Thryve

