//
// Created by kprie on 19.03.2024.
//

#pragma once
#include <functional>

#include "IRenderContext.h"
#include "Core/App.h"


namespace Thryve::Rendering {
    class Renderer {
    public:
        template <typename FunctionType>
        static void Submit(FunctionType&& func) {
            auto _renderCmd = [](std::function<FunctionType> funcPtr)
            {
                auto _pFunc = std::make_unique<FunctionType>(funcPtr);
                _pFunc();
                _pFunc.reset();
            };
        }

        static Core::SharedRef<IRenderContext> GetContext() { return Core::App::Get().GetRenderContext(); }

    private:
    };
}
