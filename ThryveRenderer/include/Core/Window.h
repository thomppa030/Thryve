//
// Created by kprie on 22.04.2024.
//
#pragma once

namespace Thryve::Rendering {
    class RenderContext;
}
struct WindowSettings {
    std::string WindowTitle{"Thryve"};
    uint32_t Width{1920};
    uint32_t Height{1080};
    bool Fullscreen{false};

    // TODO What else would we need?
};

class Window {
public:
    explicit Window(const WindowSettings& windowSpecs);
    virtual ~Window() = default;

    virtual void Init() = 0;
    virtual void GetWindow() const = 0;

    [[nodiscard]] virtual std::string& GetTitle() const = 0;
    virtual void SetTitle(const std::string& title) = 0;

    [[nodiscard]] virtual Thryve::Core::SharedRef<Thryve::Rendering::RenderContext> GetRenderContext() const = 0;

    [[nodiscard]] virtual Window* Create(const WindowSettings& settings = WindowSettings()) = 0;

protected:
    virtual void ShutDown() = 0;
};
