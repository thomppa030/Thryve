//
// Created by kprie on 15.03.2024.
//
#pragma once

#include <memory>

#include "IRenderContext.h"
#include "ThryveRenderer/include/Vulkan/VulkanRenderContext.h"

class GraphicsContext {
public:
    enum class Backend {
        Vulkan,
        OpenGL,
        DirectX
    };

    explicit GraphicsContext(const Backend backend) {
        switch (backend) {
            case Backend::Vulkan:
                // m_context = std::make_unique<Thryve::Rendering::VulkanRenderContext>();
            break;
            case Backend::OpenGL:
                // m_context = std::make_unique<OpenGLRenderContext>();
                    break;
            case Backend::DirectX:
                // m_context = std::make_unique<DirectXRenderContext>();
                    break;
        }
    }

    void run() const {
        m_context->Run();
    }

private:
    std::unique_ptr<Thryve::Rendering::IRenderContext> m_context;
};