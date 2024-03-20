//
// Created by kprie on 15.03.2024.
//
#pragma once

#include "Core/Ref.h"

namespace Thryve::Rendering {
    class IRenderContext : public Core::ReferenceCounted {
    public:
        IRenderContext() = default;
        virtual ~IRenderContext() = default;

        virtual void Run(){};
        // Add more functions as needed, such as cleanup, setting up scenes, etc.
    };
}
