//
// Created by kprie on 19.03.2024.
//
#pragma once
#include "pch.h"

namespace Thryve::Rendering {
    class RenderCommandQueue {
    public:
        using RenderCommandFn = void(*)(void *);

        RenderCommandQueue();

        ~RenderCommandQueue();

        void *Allocate(RenderCommandFn func, uint32_t size);

        void Execute();

    private:
        uint8_t *m_commandBuffer;
        uint8_t *m_commandBufferPtr;
        uint32_t m_commandCount{0};
    };
}



